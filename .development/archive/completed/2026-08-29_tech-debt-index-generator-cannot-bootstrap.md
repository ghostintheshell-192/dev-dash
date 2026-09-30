---
type: bug
priority: low
status: resolved
discovered: 2026-08-29
resolved: 2026-08-29
related: [docs-update-orchestrator-hardcodes-generated-files.md]
related_decision: 012-codebase-agnostic-automation.md
report: null
---

# `update-tech-debt-index.py` cannot create the README it maintains, unlike its two siblings

## Problem

`docs-update.sh` drives three generators. Two of them write their output file
whether or not it already exists; the third refuses:

| Generator | Output | Missing output |
| --------- | ------ | -------------- |
| `generate-architecture.sh` | `ARCHITECTURE.md` | created (`> "$OUTPUT_FILE"`) |
| `generate-index.py` | `INDEX.md` | created (`INDEX_FILE.write_text(...)`) |
| `update-tech-debt-index.py` | `tech-debt/README.md` | **`ERROR: README not found`, returns `failed`** |

So a project can regenerate two thirds of its derived documentation from an
empty `.development/` and must hand-write the third before the automation will
touch it. Nothing states that asymmetry; it is visible only by reading all three
scripts.

## Analysis

`update_readme()` starts with an existence check that returns `"failed"`
(`update-tech-debt-index.py:147`). The design reason is real: unlike the other
two, this script does not own its whole output file. It replaces one section
(`## Current Issues by Priority`) inside a README that also carries the
frontmatter schema, the workflow, and the archiving instructions — prose no
generator can reconstruct. Refusing to write is the right call *given* that
there is nothing to write into.

What is missing is the other half: a minimal skeleton to write when the file is
absent, so the contract becomes "owns a section, bootstraps the file" rather
than "owns a section, requires a file".

**Currently latent everywhere.** All five repos have the README, and the
scaffold ships both `tech-debt/README.md` and `_TEMPLATE.md`, so a freshly
scaffolded project is fine by construction. It bites in exactly three cases: a
project that adopts the generators without the scaffold, someone deleting the
file, and any future `.development/` bootstrap flow that creates directories and
expects the generators to fill them.

**How it fails is worse than that it fails.** `run_generator()` in
`docs-update.sh` discards the generator's own output (`>/dev/null 2>&1`) and
prints `docs-update: WARNING - tech-debt/README.md generator failed`. The
diagnostic naming the missing path is thrown away, the warning has no colour
among coloured success lines, and `04-docs-update` exits 0 regardless. The
commit proceeds and the README quietly never appears. That is the same silent
shape as the two issues resolved earlier today, arrived at from a different
direction.

## Possible Solutions

- **Option A**: write a minimal README when the file is absent — the H1, a line
  saying what the folder is, and the generated section — then proceed normally.
  A missing file becomes a bootstrap, not a failure. The scaffold's own
  `tech-debt/README.md` is the obvious model for the skeleton, trimmed to what
  cannot be regenerated.
- **Option B**: leave the refusal, and make it *loud*: have `run_generator()`
  surface the generator's stderr instead of swallowing it, so the operator reads
  `ERROR: README not found at ...` rather than a generic warning. Fixes the
  diagnosis, not the gap — but it fixes it for all three generators at once, and
  the swallowed output is a defect in its own right.
- **Option C**: have the scaffold-bootstrap path guarantee the file, and declare
  the generator's precondition satisfied by construction. That is already the de
  facto situation; it just is not written down anywhere, and it fails for any
  project not born from the scaffold.

## Recommended Approach

**A and B together, in that order of importance.** A closes the gap; B is three
lines in `docs-update.sh` that make every future generator failure legible
instead of a beige warning. B is worth doing even if A is deferred.

Both belong in `rsrc/project-scaffold/` first (ADR-013) and propagate from
there.

## Solution Implemented

Resolved 2026-08-29, Options A and B together, in `dev-dash` and
`rsrc/project-scaffold/`.

**A — the generator bootstraps its own file.** `update_readme()` writes a
minimal skeleton when `README.md` is absent and then proceeds normally, instead
of returning `failed`. The skeleton is deliberately thin: an H1 and two
sentences saying what the folder holds and that the frontmatter is what makes an
issue visible to the tooling. This script owns one section and cannot
reconstruct the prose around it — that was always the right reason to refuse to
*overwrite*, and never a reason to refuse to *create*. The contract is now "owns
a section, bootstraps the file"; a project wanting the full document copies it
from the scaffold. The dead `failed` branch in `main()` went with it.

**B — failures say what failed.** `run_generator()` in `docs-update.sh` captures
the generator's combined output instead of sending it to `/dev/null`, and prints
it indented under the warning when the generator exits non-zero. The warning
also goes to stderr now. Previously the one line that named the cause — a
missing path, a traceback — was discarded, leaving an operator with
`WARNING - generator failed` and nothing to act on.

**Also fixed, in the sibling issue.** The `04-docs-update` staging bug found
while testing `post-merge-does-not-regenerate-derived-docs.md`: newly *created*
generated files were never staged, because `git diff --quiet` reports no
difference for an untracked file. That was the other half of the same bootstrap
gap — this generator refusing to create its file, and the hook failing to stage
files that were created — and it is recorded in that issue.

## Testing

Against a scratch `.development/` holding one issue and no README:

- **Bootstrap**: the README is created with the skeleton, the index section
  appended, and the run reports both `Created` and `Updated`.
- **Idempotent**: the second run reports `Unchanged` and rewrites nothing.
- **Diagnostics**: with two generators deliberately absent, the entry point now
  prints `bash: ...generate-architecture.sh: No such file or directory` and
  python's `[Errno 2] No such file or directory` under their warnings, instead
  of two bare failure lines. The third generator still runs, and the entry
  point still exits 0 — a broken generator does not block a commit.

## Impact

`.development/` can now be regenerated from an empty state by the tooling that
is supposed to maintain it, which is what a project adopting these generators
without the scaffold, or any future bootstrap flow, actually needs.

## Notes

- The `GENERATED` array finding recorded below has been split out into
  `docs-update-orchestrator-hardcodes-generated-files.md` rather than archived
  along with this issue.
- Found while auditing the derived-docs pipeline for
  `post-merge-does-not-regenerate-derived-docs.md`.
- **Separate finding, deserves its own note.** `.githooks/pre-commit.d/04-docs-update`
  declares in its header that it has *"no knowledge of generators or stack"* and
  then hardcodes the three output paths in a `GENERATED=(...)` array to decide
  what to stage. A project whose `docs-update.sh` regenerates a fourth file
  would have that file regenerated and never staged. The entry point already
  prints which files it touched, so the orchestrator does not need the census —
  which makes this one more instance of a hand-kept list of something the
  system can enumerate, the pattern behind
  `architecture-layer-overview-stale-prose` and `claude-md-rules-list-incomplete`
  as well.

## Related Documentation

- **Related Issues**: `post-merge-does-not-regenerate-derived-docs.md` — same
  pipeline, and the reason this was found
- **Architecture Decision**: `012-codebase-agnostic-automation.md` — the entry
  point contract this sits inside; `013-scaffold-source-of-truth.md` for
  propagation order
- **Code Locations**:
  - `.development/scripts/update-tech-debt-index.py` — `update_readme()`, the
    existence check at line 147
  - `.development/automation/docs-update.sh` — `run_generator()`, the discarded
    output
  - `.githooks/pre-commit.d/04-docs-update` — the `GENERATED` array
  - `rsrc/project-scaffold/.development/tech-debt/README.md` — the shipped file
    that keeps this latent, and the model for a skeleton
