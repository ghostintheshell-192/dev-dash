"""Extract classes and relations from app/src with libclang (semantic)."""
import json
import os
import shlex
import subprocess
import sys
import time

import clang.cindex as ci

from model import ClassInfo, Member, Relation

CK = ci.CursorKind
TK = ci.TypeKind

SRC_ROOT = os.path.realpath(sys.argv[1])        # .../app/src
CC_PATH = sys.argv[2]                            # compile_commands.json

# The pip wheel of libclang ships without the compiler's own headers
# (stddef.h, stdarg.h...): without them every translation unit stops at the
# first libc include. Borrow GCC's.
GCC_BUILTIN_INCLUDE = subprocess.run(["gcc", "-print-file-name=include"],
                                     capture_output=True, text=True).stdout.strip()

ACCESS = {ci.AccessSpecifier.PUBLIC: "public",
          ci.AccessSpecifier.PROTECTED: "protected",
          ci.AccessSpecifier.PRIVATE: "private"}

# Wrappers that mean "owns a T" (composition) vs "refers to a T" (aggregation).
OWNING = {"unique_ptr", "optional", "vector", "array", "deque", "list", "map",
          "unordered_map", "set", "shared_ptr"}


def in_project(cursor):
    loc = cursor.location
    return loc.file is not None and os.path.realpath(loc.file.name).startswith(SRC_ROOT)


def qualified(cursor):
    parts = []
    c = cursor
    while c is not None and c.kind != CK.TRANSLATION_UNIT:
        if c.spelling:
            parts.append(c.spelling)
        c = c.semantic_parent
    return "::".join(reversed(parts))


def record_targets(t, depth=0):
    """Yield (qualified record name, how) reachable from a type: how is 'value'
    for direct/owning use and 'ref' for references and raw pointers."""
    if depth > 6:
        return
    t = t.get_canonical() if t.kind == TK.ELABORATED else t
    if t.kind in (TK.LVALUEREFERENCE, TK.RVALUEREFERENCE, TK.POINTER):
        for name, _ in record_targets(t.get_pointee(), depth + 1):
            yield name, "ref"
        return
    decl = t.get_declaration()
    if decl.kind in (CK.CLASS_DECL, CK.STRUCT_DECL, CK.CLASS_TEMPLATE,
                     CK.TYPEDEF_DECL, CK.TYPE_ALIAS_DECL) and decl.spelling:
        if in_project(decl) and decl.kind in (CK.CLASS_DECL, CK.STRUCT_DECL):
            yield qualified(decl), "value"
        n = t.get_num_template_arguments()
        # Only the arguments that carry meaning: owning wrappers hold their first
        # argument (maps also their second); the rest are allocators, deleters,
        # comparators and traits.
        if decl.spelling in OWNING:
            n = min(n, 2 if "map" in decl.spelling else 1)
        for i in range(max(n, 0)):
            arg = t.get_template_argument_type(i)
            if arg.kind != TK.INVALID:
                for name, how in record_targets(arg, depth + 1):
                    yield name, how if decl.spelling in OWNING else "ref"
        if decl.kind in (CK.TYPEDEF_DECL, CK.TYPE_ALIAS_DECL):
            yield from record_targets(decl.underlying_typedef_type, depth + 1)


def main():
    index = ci.Index.create()
    commands = json.load(open(CC_PATH))
    tus = [c for c in commands if os.path.realpath(c["file"]).startswith(SRC_ROOT)]

    classes = {}
    relations = set()
    diagnostics = 0
    t0 = time.perf_counter()

    for cmd in tus:
        args = shlex.split(cmd["command"])[1:]
        # drop the output/input and the -c flag; keep defines, includes, std
        clean, skip = [], False
        for a in args:
            if skip:
                skip = False
                continue
            if a in ("-o", "-MF", "-MT", "-MQ"):
                skip = True
                continue
            if a in ("-c",) or a == cmd["file"] or a.endswith(".cpp"):
                continue
            clean.append(a)
        clean += ["-isystem", GCC_BUILTIN_INCLUDE]
        os.chdir(cmd["directory"])
        tu = index.parse(cmd["file"], args=clean,
                         options=ci.TranslationUnit.PARSE_SKIP_FUNCTION_BODIES)
        diagnostics += sum(1 for d in tu.diagnostics if d.severity >= ci.Diagnostic.Error)

        for cur in tu.cursor.walk_preorder():
            if cur.kind not in (CK.CLASS_DECL, CK.STRUCT_DECL, CK.CLASS_TEMPLATE):
                continue
            if not cur.is_definition() or not in_project(cur):
                continue
            q = qualified(cur)
            if q in classes:
                continue
            info = ClassInfo(q, os.path.relpath(cur.location.file.name, SRC_ROOT),
                             "struct" if cur.kind == CK.STRUCT_DECL else "class")
            for ch in cur.get_children():
                if ch.kind == CK.CXX_BASE_SPECIFIER:
                    base = ch.type.get_declaration()
                    bname = qualified(base) if base.spelling else ch.type.spelling
                    info.bases.append(bname)
                    relations.add(Relation(q, bname, "inherits"))
                elif ch.kind == CK.FIELD_DECL:
                    info.members.append(Member(ch.spelling, ch.type.spelling,
                                               ACCESS.get(ch.access_specifier, "?"), False))
                    for name, how in record_targets(ch.type):
                        if name != q:
                            relations.add(Relation(q, name, "composes" if how == "value" else "aggregates"))
                elif ch.kind in (CK.CXX_METHOD, CK.CONSTRUCTOR, CK.DESTRUCTOR, CK.FUNCTION_TEMPLATE):
                    info.members.append(Member(ch.spelling, ch.type.spelling,
                                               ACCESS.get(ch.access_specifier, "?"), True))
                    types = [ch.result_type] + [a.type for a in ch.get_arguments()]
                    for t in types:
                        for name, _ in record_targets(t):
                            if name != q:
                                relations.add(Relation(q, name, "depends"))
            classes[q] = info

    elapsed = time.perf_counter() - t0
    # A member relation outranks a signature dependency on the same pair.
    rank = {"inherits": 0, "composes": 1, "aggregates": 2, "depends": 3}
    best = {}
    for r in relations:
        if (r.src, r.dst) not in best or rank[r.kind] < rank[best[(r.src, r.dst)].kind]:
            best[(r.src, r.dst)] = r
    relations = set(best.values())
    json.dump({
        "extractor": "libclang",
        "seconds": round(elapsed, 2),
        "translation_units": len(tus),
        "parse_errors": diagnostics,
        "classes": [c.__dict__ | {"members": [m.__dict__ for m in c.members]} for c in classes.values()],
        "relations": sorted([r.__dict__ for r in relations], key=lambda r: (r["src"], r["dst"], r["kind"])),
    }, sys.stdout, indent=1)


main()
