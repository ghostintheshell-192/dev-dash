---
type: bug
priority: medium
status: resolved
discovered: 2026-07-24
resolved: 2026-07-25
related: [scaffold-architecture-eval-glob-expansion.md]
related_decision: null
report: null
---

# Doc generators rewrite their output on every run (INDEX.md, tech-debt/README.md)

## Problem

`generate-index.py` and `update-tech-debt-index.py` write their output
unconditionally, and the output embeds a wall-clock timestamp:

```python
lines.append(f"*Auto-generated: {now.strftime('%Y-%m-%d %H:%M')}*")
```

So two runs a minute apart produce a diff even when nothing in the project
changed. On its own that is cosmetic. Combined with `04-docs-update`, which
regenerates and **stages** the derived docs on every commit, it stops being
cosmetic: *every commit in the project* carries one or two files of pure noise.
The cost is not disk, it is attention — a diff that is always dirty is a diff
people stop reading.

`generate-index.py` has a second, worse instance of the same shape:

- It indexes `.development/`, which contains `INDEX.md` itself.
- Entries are sorted by raw `mtime`, precise to the second.
- Writing `INDEX.md` bumps its own mtime, so the *next* run reads a different
  mtime and reorders the entries around it.

The result oscillates between runs with no change behind it:

```diff
-2. [README.md](README.md) (today)
-3. [INDEX.md](INDEX.md) (today)
+2. [INDEX.md](INDEX.md) (today)
+3. [README.md](README.md) (today)
```

Note the mismatch that makes it possible: the sort is second-precise, but every
entry is *rendered* only to the day (`today`, `2d ago`). The generator sorts on
information it never shows, so the reordering is unaccountable to the reader.

## Fix applied in raid-sandbox

Both parts, minimally, without dropping the timestamp (it is informative; the
unconditional write is the defect):

1. **Write only on real change.** Each script compares current vs new content
   with the timestamp line stripped, and returns early when they match:

   ```python
   def strip_timestamp(text: str) -> str:
       return "\n".join(
           line for line in text.splitlines()
           if not line.startswith(TIMESTAMP_PREFIX)
       )
   ```

2. **Sort at the granularity that is displayed.** A shared `recency_key`
   replaces `key=lambda x: x["mtime"], reverse=True` at both call sites:

   ```python
   def recency_key(file_info: dict) -> tuple:
       return (-file_info["mtime"].toordinal(), file_info["name"])
   ```

   Same information on screen, total order, no churn. `INDEX.md` keeps
   indexing itself — the self-reference is fine once the ordering no longer
   depends on sub-day mtime.

Verified with three consecutive `docs-update.sh` runs: the first stabilises the
ordering, the next two leave both files byte-identical.

Commit: `fix(scripts): make the doc generators idempotent` on
`raid-sandbox@chore/scaffold-alignment`.

## Next step

Port both changes back into the scaffold's copies, so raid-sandbox stops being
a divergence. Any project that adopts `04-docs-update` hits this on its first
commit.

## Resolution (2026-07-25)

Both changes ported into the scaffold and the dev-dash live copies, converging
on the raid-sandbox implementation: write-only-on-real-change guarded by a
`strip_timestamp()` comparison, and a shared `recency_key()` replacing the
sub-day `mtime` sort at both call sites in `generate-index.py`.

Verified over six consecutive runs. `INDEX.md` reached its fixed point at run 2
and both files stayed byte-identical through run 6, with mtimes frozen after
run 2 — no writes, and no period-2 oscillation. Reproduced on a clean copy of
the scaffold scripts. Content correctness checked as well, not just stability:
the regenerated `tech-debt/README.md` picked up seven issues it had been
missing, and `INDEX.md`'s self-entry was stale.

Confirmed live during the commits that followed: the archive hook's
`docs-update` step reported `Derived docs already current` instead of restaging
noise.

### Corrections to the analysis above

- **"The first run stabilises the ordering, the next two leave both files
  byte-identical"** is not accurate. `INDEX.md` needs **two** runs to reach its
  fixed point, because it indexes itself and records its own `size_kb` and
  mtime: run 1 captures the pre-write values, run 2 the post-write ones, and
  run 3 is the first true no-op. This is inherent to the self-indexing design,
  not specific to dev-dash — it reproduced on a clean copy.
- **"Same information on screen"** overstates it. `recency_key()` orders by
  calendar day (`toordinal()`), while `format_file_entry` and the "Recently
  Modified" block label entries using a rolling 24-hour window
  (`(now - mtime).days`). A file touched yesterday at 23:11 is therefore
  labelled "today" but sorted with yesterday's group. It produces no churn, so
  it is not a regression, but the sort key and the label are not the same
  notion of "day".

### Known warts, deliberately left

Both are present in the raid-sandbox reference and were kept rather than
diverging silently:

- `update_readme()` returns `True` on the no-write path, so `main()` prints
  `Updated <path>` even when nothing was written. `generate-index.py` correctly
  prints `Unchanged` in the same situation. Worth cleaning up across all three
  repos in one pass.
- The day-granularity mismatch described above.
