"""Compare the two extractions: classes, bases, members and relations."""
import json
import sys
from collections import Counter

a = json.load(open(sys.argv[1]))   # libclang
b = json.load(open(sys.argv[2]))   # tree-sitter

print(f"time: libclang {a['seconds']}s, tree-sitter {b['seconds']}s")
ca = {c["name"]: c for c in a["classes"]}
cb = {c["name"]: c for c in b["classes"]}
print(f"classes: libclang {len(ca)}, tree-sitter {len(cb)}, common {len(ca.keys() & cb.keys())}")
for n in sorted(ca.keys() - cb.keys()):
    print("  only libclang:", n, ca[n]["file"])
for n in sorted(cb.keys() - ca.keys()):
    print("  only tree-sitter:", n, cb[n]["file"])

mem_diff = 0
for n in sorted(ca.keys() & cb.keys()):
    ma = {(m["name"], m["is_method"], m["access"]) for m in ca[n]["members"]}
    mb = {(m["name"], m["is_method"], m["access"]) for m in cb[n]["members"]}
    if ma != mb:
        mem_diff += 1
        print(f"  members differ in {n}: only clang {sorted(ma - mb)[:6]} | only ts {sorted(mb - ma)[:6]}")
print(f"classes with different member sets: {mem_diff}")

ra = {(r["src"], r["dst"], r["kind"]) for r in a["relations"]}
rb = {(r["src"], r["dst"], r["kind"]) for r in b["relations"]}
print(f"relations: libclang {len(ra)} {dict(Counter(k for *_, k in ra))}")
print(f"           tree-sitter {len(rb)} {dict(Counter(k for *_, k in rb))}")
print(f"           identical {len(ra & rb)}")
pa = {(s, d) for s, d, _ in ra}
pb = {(s, d) for s, d, _ in rb}
kind_a = {(s, d): k for s, d, k in ra}
kind_b = {(s, d): k for s, d, k in rb}
same_pair_diff_kind = sorted(p for p in pa & pb if kind_a[p] != kind_b[p])
print(f"same pair, different kind: {len(same_pair_diff_kind)}")
for p in same_pair_diff_kind:
    print(f"  {p[0]} -> {p[1]}: clang {kind_a[p]}, ts {kind_b[p]}")
print(f"pairs only libclang: {len(pa - pb)}")
for p in sorted(pa - pb):
    print(f"  {p[0]} -> {p[1]} ({kind_a[p]})")
print(f"pairs only tree-sitter: {len(pb - pa)}")
for p in sorted(pb - pa):
    print(f"  {p[0]} -> {p[1]} ({kind_b[p]})")
