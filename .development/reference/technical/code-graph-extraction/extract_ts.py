"""Extract classes and relations from app/src with tree-sitter (syntax only).

Names are resolved textually: a type mentions class X if one of its
identifiers equals the short name of a class found anywhere in the project.
Namespaces are tracked from the enclosing namespace blocks."""
import json
import os
import re
import sys
import time

import tree_sitter_cpp
from tree_sitter import Language, Parser

from model import ClassInfo, Member, Relation, short

SRC_ROOT = os.path.realpath(sys.argv[1])
LANG = Language(tree_sitter_cpp.language())
parser = Parser(LANG)

OWNING = {"unique_ptr", "optional", "vector", "array", "deque", "list", "map",
          "unordered_map", "set", "shared_ptr"}


def text(node):
    return node.text.decode("utf-8")


def walk_classes(node, ns, outer, out, rel_path):
    """Collect class/struct definitions with their namespace path."""
    for ch in node.children:
        if ch.type == "namespace_definition":
            name = ch.child_by_field_name("name")
            body = ch.child_by_field_name("body")
            parts = text(name).split("::") if name else []
            if body:
                walk_classes(body, ns + parts, [], out, rel_path)
        elif ch.type in ("class_specifier", "struct_specifier") and ch.child_by_field_name("body"):
            name_node = ch.child_by_field_name("name")
            if name_node is None:
                continue
            q = "::".join(ns + outer + [text(name_node)])
            out.append((q, ch, rel_path))
            walk_classes(ch.child_by_field_name("body"), ns, outer + [text(name_node)], out, rel_path)
        elif ch.type in ("declaration", "field_declaration", "template_declaration",
                         "declaration_list", "linkage_specification", "type_definition"):
            walk_classes(ch, ns, outer, out, rel_path)


def declarator_name(decl):
    """Innermost identifier of a (possibly pointer/reference/function) declarator."""
    while decl is not None and decl.type not in ("identifier", "field_identifier",
                                                  "destructor_name", "operator_name",
                                                  "qualified_identifier"):
        decl = decl.child_by_field_name("declarator") or (decl.named_children[0] if decl.named_children else None)
    return text(decl) if decl is not None else "?"


def inner(decl):
    """Next declarator down; reference declarators keep theirs as an unnamed child."""
    nxt = decl.child_by_field_name("declarator")
    if nxt is None and decl.type == "reference_declarator" and decl.named_children:
        nxt = decl.named_children[0]
    return nxt


def is_function(decl):
    while decl is not None:
        if decl.type == "function_declarator":
            return True
        decl = inner(decl)
    return False


def ref_like(decl):
    while decl is not None:
        if decl.type in ("reference_declarator", "pointer_declarator"):
            return True
        if decl.type == "function_declarator":
            return False
        decl = decl.child_by_field_name("declarator") or (
            decl.named_children[0] if decl.type in ("reference_declarator",) and decl.named_children else None)
    return False


def main():
    t0 = time.perf_counter()
    found = []
    errors = 0
    for root, _, files in os.walk(SRC_ROOT):
        for f in sorted(files):
            if not f.endswith((".h", ".hpp", ".cpp")):
                continue
            path = os.path.join(root, f)
            tree = parser.parse(open(path, "rb").read())
            errors += int(tree.root_node.has_error)
            walk_classes(tree.root_node, [], [], found, os.path.relpath(path, SRC_ROOT))

    classes = {}
    for q, node, rel in found:
        if q in classes:
            continue
        classes[q] = (ClassInfo(q, rel, "struct" if node.type == "struct_specifier" else "class"), node)
    by_short = {}
    for q in classes:
        by_short.setdefault(short(q), []).append(q)

    def mentions(type_text, owner):
        """Classes named by a type, with 'value' or 'ref' like the clang side."""
        idents = re.findall(r"[A-Za-z_]\w*", type_text)
        wrapper_owning = any(i in OWNING for i in idents)
        res = []
        for i in idents:
            for cand in by_short.get(i, []):
                if cand != owner:
                    # prefer a candidate in the owner's namespace when ambiguous
                    res.append(cand)
        return res, wrapper_owning

    relations = set()
    for q, (info, node) in classes.items():
        access = "public" if info.kind == "struct" else "private"
        base_clause = next((c for c in node.children if c.type == "base_class_clause"), None)
        if base_clause:
            for b in base_clause.named_children:
                if b.type in ("type_identifier", "qualified_identifier", "template_type"):
                    info.bases.append(text(b))
                    for cand in by_short.get(short(text(b)), []) or [text(b)]:
                        relations.add(Relation(q, cand, "inherits"))
        for ch in node.child_by_field_name("body").children:
            if ch.type == "access_specifier":
                access = text(ch).rstrip(":").strip()
                continue
            if ch.type not in ("field_declaration", "declaration", "function_definition"):
                continue
            tnode = ch.child_by_field_name("type")
            decl = ch.child_by_field_name("declarator")
            if decl is None:
                continue  # nested class/struct definitions, handled separately
            ttext = text(tnode) if tnode else ""
            method = is_function(decl)
            info.members.append(Member(declarator_name(decl), ttext, access, method))
            if method:
                sig = ttext + " " + text(decl)
                for cand in mentions(sig, q)[0]:
                    relations.add(Relation(q, cand, "depends"))
            else:
                targets, owning = mentions(ttext, q)
                ref = ref_like(decl) or ("&" in text(decl)) or ("*" in text(decl) and not owning)
                for cand in targets:
                    relations.add(Relation(q, cand, "aggregates" if ref and not owning else "composes"))

    rank = {"inherits": 0, "composes": 1, "aggregates": 2, "depends": 3}
    best = {}
    for r in relations:
        if (r.src, r.dst) not in best or rank[r.kind] < rank[best[(r.src, r.dst)].kind]:
            best[(r.src, r.dst)] = r
    relations = set(best.values())
    elapsed = time.perf_counter() - t0
    json.dump({
        "extractor": "tree-sitter",
        "seconds": round(elapsed, 2),
        "files_with_syntax_errors": errors,
        "classes": [c.__dict__ | {"members": [m.__dict__ for m in c.members]} for c, _ in classes.values()],
        "relations": sorted([r.__dict__ for r in relations], key=lambda r: (r["src"], r["dst"], r["kind"])),
    }, sys.stdout, indent=1)


main()
