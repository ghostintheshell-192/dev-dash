---
type: bug
priority: high
status: resolved
discovered: 2026-07-25
resolved: 2026-07-25
related: [scaffold-doc-generators-not-idempotent.md]
related_decision: null
report: null
---

# 03-archive-resolved-issues commits the archived copy but never the removal of the original

## Problem

When a tech-debt issue is staged with `status: resolved`, the pre-commit hook
`03-archive-resolved-issues` moves it to `archive/completed/` and stages the new
path — but the **removal of the original path is never staged**. The resulting
commit contains the issue in *both* locations, while `tech-debt/README.md`
(regenerated afterwards by `04-docs-update`, which reads the filesystem) already
reports it as archived.

The commit is therefore internally inconsistent: the index says the issue is
still open in `tech-debt/`, the derived index says it is archived, and the
working tree is left carrying an unstaged deletion that nothing will pick up.

Observed live on 2026-07-25 while landing
`scaffold-architecture-eval-glob-expansion.md`:

```
$ git ls-tree --name-only HEAD .development/tech-debt/scaffold-architecture-eval-glob-expansion.md
.development/tech-debt/scaffold-architecture-eval-glob-expansion.md   # still tracked
$ ls .development/tech-debt/scaffold-architecture-eval-glob-expansion.md
ls: no such file or directory                                          # gone from disk
```

**This is not the first occurrence.** `.development/tech-debt/scaffold-architecture-scripts.md`
was archived on 2026-06-28 and left exactly the same dangling deletion, which sat
uncommitted in the working tree for a month and was initially mistaken for
unrelated leftover state. The failure is silent enough to survive a full release
cycle unnoticed.

## Analysis

Root cause is three lines in `.githooks/pre-commit.d/03-archive-resolved-issues`:

```bash
mv "$FULL_PATH" "$NEW_PATH"

# Update git staging
git reset HEAD "$file" 2>/dev/null || true
git add "$NEW_PATH"
```

The issue file reaches the hook staged as a **modification** (the commit that
resolves it edits its frontmatter). `git reset HEAD "$file"` restores that path
in the index to its HEAD state — it does not remove it. Since `mv` has already
taken the file off disk, the net effect is:

- index: original path present, at its pre-resolution content
- worktree: original path absent
- index: archived path present (staged by the following `git add`)

so `git status` reports an unstaged `D` that the commit does not include.

The intent behind the `git reset` was presumably "unstage the old path", which
is what it does — but unstaging is the wrong operation. The index needs to
*record* the removal, not forget the change.

Two adjacent weaknesses in the same block:

- The hook uses `mv` rather than `git mv`, so git only infers the rename
  heuristically once both sides are staged. Not a defect on its own, but it is
  why the operation does not read as a rename in the commit.
- Unlike the manual `archive-resolved-issues.sh`, the hook has **no
  target-exists guard**: if an archived file with the same dated name already
  exists, `mv` overwrites it silently. The manual script skips and warns.

The standalone `.development/scripts/archive-resolved-issues.sh` does not touch
git at all, leaving staging entirely to the caller. That is defensible for a
manual tool, but it means the same duplicated-in-git state can be produced by
hand and committed without noticing.

## Possible Solutions

- **Option A — stage both sides of the move (recommended)**: replace the
  `reset`/`add` pair with a single `git add -A` over both paths, which records
  the deletion of the old path and the addition of the new one:

  ```bash
  mv "$FULL_PATH" "$NEW_PATH"
  git add -A -- "$file" "$NEW_PATH"
  ```

- **Option B — `git rm --cached` the original**: keeps the two-step shape,
  swapping `git reset HEAD "$file"` for `git rm --cached --quiet -- "$file"`.
  Equivalent outcome, marginally more explicit about intent, one more command.

- **Option C — `git mv`**: let git perform the move so both sides are staged
  atomically. Cleanest in principle, but it fails when the target directory is
  not yet created and needs the same `mkdir -p` dance, and it changes more of
  the surrounding code than the defect warrants.

## Recommended Approach

**Option A.** One line, fixes the actual defect (the index must record the
removal), and keeps the hook's existing structure. Add the target-exists guard
from the manual script in the same pass, since both live in the same block and
both are silent-overwrite risks.

Once the hook is fixed, the two commits already affected are worth checking:
2026-06-28 (`scaffold-architecture-scripts.md`) and 2026-07-25
(`scaffold-architecture-eval-glob-expansion.md`). Both were corrected by hand
after the fact, so no content was lost.

## Notes

- The scaffold copy at
  `rsrc/project-scaffold/.githooks/pre-commit.d/03-archive-resolved-issues`
  carries the same code and must be fixed in the same pass — every scaffolded
  project inherits this.
- The write-up content itself is never at risk: the archived copy is the moved
  file, so it carries the resolution text in full. What is corrupted is the
  commit's shape, not the documentation.
- Worth considering a cheap invariant check somewhere in the chain: after the
  archive step, no path should exist both in `tech-debt/` (per the index) and in
  `archive/completed/`. That would have caught the June instance the same day.

## Resolution (2026-07-25)

Option A applied to both copies of the hook:

- `git reset HEAD "$file"` + `git add "$NEW_PATH"` replaced by a single
  `git add -A -- "$FULL_PATH" "$NEW_PATH"`, which records the removal of the
  original and the addition of the archived copy in one step.
- Target-exists guard added, mirroring the manual script: an archived file with
  the same dated name is no longer silently overwritten — the issue is skipped
  with a warning.

### Self-test, and its limit

This write-up was marked `resolved` in the same commit that fixes the hook, so
the **fixed** hook is the one that archived it. The outcome was correct: only
the archived path in the commit, nothing tracked in two places, no dangling
deletion in the working tree.

That is weaker evidence than it looks, and the distinction matters. This file
was **new** — staged as `A`, not `M`. The defect is specific to the `M` case:
`git reset HEAD` on a modified path restores it in the index at its HEAD
content, which is what leaves it tracked. On an added path the same reset simply
drops it from the index, so the *old* hook would have produced a correct result
here too. This commit therefore exercised the fix, not the failure.

Real verification requires resolving an issue that is already tracked. The next
commit in the queue does exactly that for two issues at once
(`tech-debt-index-backslash-escape.md` and
`scaffold-doc-generators-not-idempotent.md`, both committed on 2026-07-25), and
is the first genuine test of this fix.

### Prior occurrences

Both were corrected by hand and no content was lost:

- 2026-06-28 — `scaffold-architecture-scripts.md`; the dangling deletion sat
  uncommitted for a month.
- 2026-07-25 — `scaffold-architecture-eval-glob-expansion.md`; caught during the
  same session that produced this write-up, corrected with `git add -A` plus an
  amend, after which git recognised the move as a rename.

The invariant suggested in Notes — no path present both in `tech-debt/` per the
index and in `archive/completed/` — is **not** implemented. It remains the cheap
check that would have caught the June instance the same day.

## Related Documentation

- **Code Locations**:
  - `.githooks/pre-commit.d/03-archive-resolved-issues` — the `mv` /
    `git reset` / `git add` block
  - `rsrc/project-scaffold/.githooks/pre-commit.d/03-archive-resolved-issues`
    (same code)
  - `.development/scripts/archive-resolved-issues.sh` — manual path; has the
    target-exists guard the hook lacks, touches git not at all
- **Prior occurrence**: commit archiving `scaffold-architecture-scripts.md`
  (2026-06-28) and the month-long dangling deletion that followed

---

📍 **Investigation Note**: Read [ARCHITECTURE.md](../ARCHITECTURE.md) to locate relevant files and understand the architectural context before starting your analysis.
