"""tree-sitter extractor, second version: with our own name resolution.

tree-sitter only gives the syntax tree. Everything that links a type name to
the class it means is ours, and this version implements a simplified form of
C++ name lookup on top of it:

  - qualified names are looked up as written (b::Node is b::Node, not "Node");
  - lookup walks the enclosing scopes from the innermost outwards;
  - `using X = T;`, `typedef T X;` and alias templates are expanded;
  - `using ns::Name;` and `using namespace ns;` are honoured in their scope;
  - template arguments are stripped from base classes (CRTP).

What stays out of reach: macros (no preprocessor), and anything that needs the
compiler (overloads, template instantiation, types from external headers)."""
import json
import os
import re
import sys
import time

import tree_sitter_cpp
from tree_sitter import Language, Parser

from model import ClassInfo, Member, Relation

SRC_ROOT = os.path.realpath(sys.argv[1])
parser = Parser(Language(tree_sitter_cpp.language()))

OWNING = {"unique_ptr", "optional", "vector", "array", "deque", "list", "map",
          "unordered_map", "set", "shared_ptr"}
NAME_RE = re.compile(r"(?:\w+::)*\w+")


def text(node):
    return node.text.decode("utf-8")


class Index:
    """Everything name lookup needs, keyed by scope ("" is the global scope)."""

    def __init__(self):
        self.classes = {}        # qualified name -> (ClassInfo, node, scope)
        self.aliases = {}        # qualified alias -> (body text, template params, scope)
        self.using_decls = {}    # scope -> {short name: (target text, scope)}
        self.using_dirs = {}     # scope -> [namespace text]


def join(scope, name):
    return f"{scope}::{name}" if scope else name


def collect(node, scope, idx, rel_path, template_params=()):
    for ch in node.children:
        t = ch.type
        if t == "namespace_definition":
            name = ch.child_by_field_name("name")
            body = ch.child_by_field_name("body")
            if body:
                collect(body, join(scope, text(name)) if name else scope, idx, rel_path)
        elif t in ("class_specifier", "struct_specifier") and ch.child_by_field_name("body"):
            name = ch.child_by_field_name("name")
            if name is None:
                continue
            q = join(scope, text(name))
            if q not in idx.classes:
                kind = "struct" if t == "struct_specifier" else "class"
                idx.classes[q] = (ClassInfo(q, rel_path, kind), ch, scope)
            collect(ch.child_by_field_name("body"), q, idx, rel_path)
        elif t == "template_declaration":
            params = tuple(text(p.child_by_field_name("name") or p.named_children[-1])
                           for p in (ch.child_by_field_name("parameters").named_children
                                     if ch.child_by_field_name("parameters") else []))
            collect(ch, scope, idx, rel_path, params)
        elif t == "alias_declaration":
            name = ch.child_by_field_name("name")
            body = ch.child_by_field_name("type")
            idx.aliases[join(scope, text(name))] = (text(body), template_params, scope)
        elif t == "type_definition":
            body = ch.child_by_field_name("type")
            for d in ch.children_by_field_name("declarator"):
                idx.aliases[join(scope, text(d))] = (text(body), (), scope)
        elif t == "using_declaration":
            raw = text(ch)
            if raw.startswith("using namespace"):
                target = raw[len("using namespace"):].strip(" ;")
                idx.using_dirs.setdefault(scope, []).append(target)
            else:
                target = raw[len("using"):].strip(" ;")
                idx.using_decls.setdefault(scope, {})[target.split("::")[-1]] = (target, scope)
        elif t in ("declaration", "field_declaration", "declaration_list",
                   "linkage_specification", "field_declaration_list"):
            collect(ch, scope, idx, rel_path, template_params)


def enclosing(scope):
    """The scope itself, then each enclosing scope, ending with global."""
    parts = scope.split("::") if scope else []
    for i in range(len(parts), -1, -1):
        yield "::".join(parts[:i])


def lookup(name, scope, idx, depth=0):
    """Resolve a (possibly qualified) name seen in `scope`.
    Returns ('class', q) | ('alias', q) | None."""
    if depth > 8:
        return None
    first, _, rest = name.partition("::")
    for s in enclosing(scope):
        cand = join(s, name)
        if cand in idx.classes:
            return ("class", cand)
        if cand in idx.aliases:
            return ("alias", cand)
        # using-declaration of the first component in this scope
        ud = idx.using_decls.get(s, {}).get(first)
        if ud:
            target = ud[0] + ("::" + rest if rest else "")
            return lookup(target, ud[1], idx, depth + 1)
        # using-directives: look inside each nominated namespace
        for ns in idx.using_dirs.get(s, []):
            for inner in (join(ns, name), join(join(s, ns), name)):
                if inner in idx.classes:
                    return ("class", inner)
                if inner in idx.aliases:
                    return ("alias", inner)
    return None


def mentions(type_text, scope, idx, depth=0):
    """Project classes named in a type, and whether an owning wrapper is involved.
    Returns ({class: owning?}, ambiguous names)."""
    found, owning_any = {}, False
    if depth > 8:
        return found, owning_any
    for name in NAME_RE.findall(type_text):
        if name.split("::")[-1] in OWNING:
            owning_any = True
            continue
        hit = lookup(name, scope, idx)
        if hit is None:
            continue
        kind, q = hit
        if kind == "class":
            found[q] = True
        else:
            body, params, ascope = idx.aliases[q]
            # template parameters of the alias are placeholders, drop them
            cleaned = " ".join(n for n in NAME_RE.findall(body) if n not in params)
            sub, sub_owning = mentions(cleaned, ascope, idx, depth + 1)
            found.update(sub)
            owning_any = owning_any or sub_owning
    return found, owning_any


def inner(decl):
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


def declarator_name(decl):
    while decl is not None and decl.type not in ("identifier", "field_identifier", "destructor_name",
                                                  "operator_name", "qualified_identifier"):
        decl = inner(decl) or (decl.named_children[0] if decl.named_children else None)
    return text(decl) if decl is not None else "?"


def main():
    t0 = time.perf_counter()
    idx = Index()
    errors = 0
    for root, _, files in os.walk(SRC_ROOT):
        for f in sorted(files):
            if f.endswith((".h", ".hpp", ".cpp")):
                path = os.path.join(root, f)
                tree = parser.parse(open(path, "rb").read())
                errors += int(tree.root_node.has_error)
                collect(tree.root_node, "", idx, os.path.relpath(path, SRC_ROOT))

    relations = set()
    for q, (info, node, _scope) in idx.classes.items():
        access = "public" if info.kind == "struct" else "private"
        base_clause = next((c for c in node.children if c.type == "base_class_clause"), None)
        if base_clause:
            for b in base_clause.named_children:
                if b.type in ("type_identifier", "qualified_identifier", "template_type"):
                    raw = text(b)
                    info.bases.append(raw)
                    bare = raw.split("<")[0]            # CRTP: Base<Derived> -> Base
                    hit = lookup(bare, q, idx)
                    relations.add(Relation(q, hit[1] if hit else bare, "inherits"))
        for ch in node.child_by_field_name("body").children:
            if ch.type == "access_specifier":
                access = text(ch).rstrip(":").strip()
                continue
            if ch.type not in ("field_declaration", "declaration", "function_definition"):
                continue
            tnode = ch.child_by_field_name("type")
            decl = ch.child_by_field_name("declarator")
            if decl is None:
                continue
            ttext = text(tnode) if tnode else ""
            method = is_function(decl)
            info.members.append(Member(declarator_name(decl), ttext, access, method))
            if method:
                targets, _ = mentions(ttext + " " + text(decl), q, idx)
                for cand in targets:
                    if cand != q:
                        relations.add(Relation(q, cand, "depends"))
            else:
                targets, owning = mentions(ttext, q, idx)
                ref = "&" in text(decl) or ("*" in text(decl) and not owning)
                for cand in targets:
                    if cand != q:
                        relations.add(Relation(q, cand, "aggregates" if ref and not owning else "composes"))

    rank = {"inherits": 0, "composes": 1, "aggregates": 2, "depends": 3}
    best = {}
    for r in relations:
        if (r.src, r.dst) not in best or rank[r.kind] < rank[best[(r.src, r.dst)].kind]:
            best[(r.src, r.dst)] = r
    json.dump({
        "extractor": "tree-sitter + resolver",
        "seconds": round(time.perf_counter() - t0, 3),
        "files_with_syntax_errors": errors,
        "classes": [c.__dict__ | {"members": [m.__dict__ for m in c.members]} for c, _, _ in idx.classes.values()],
        "relations": sorted([r.__dict__ for r in best.values()], key=lambda r: (r["src"], r["dst"], r["kind"])),
    }, sys.stdout, indent=1)


main()
