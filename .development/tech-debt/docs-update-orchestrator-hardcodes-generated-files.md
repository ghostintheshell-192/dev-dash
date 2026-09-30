---
type: code-quality
priority: low
status: open
discovered: 2026-08-29
resolved: null
related: []
related_decision: 012-codebase-agnostic-automation.md
report: null
---

# `04-docs-update` declares it knows nothing about the generators, then hardcodes their outputs

## Problem

The module's own header says it *"delegates to the project's docs-update entry
point and stages whatever generated files changed. No knowledge of generators
or stack."* Four lines below the staging loop sits:

```bash
GENERATED=(
    ".development/ARCHITECTURE.md"
    ".development/INDEX.md"
    ".development/tech-debt/README.md"
)
```

That is knowledge of the generators, in the file that claims not to have any. A
project whose `docs-update.sh` regenerates a fourth file gets it regenerated on
every commit and staged on none — the change sits in the working tree, the hook
reports success, and the file drifts out of the commit silently.

## Analysis

The entry point already publishes what it touched. Its contract, stated at the
top of `docs-update.sh`, is *"idempotent; always exit 0 unless a generator
itself crashes. Prints which files it touched"*, and it emits one
`docs-update: <path> regenerated` line per generator, with `<path>` relative to
`.development/`. The orchestrator has the information available and duplicates
it by hand instead.

Nothing is broken today: all five repos generate exactly those three files, and
the array is correct. It is a latent defect that becomes real the first time a
generator is added — which is precisely when nobody will be looking at a hook
that has worked for months.

This is the same shape as `architecture-layer-overview-stale-prose.md` and
`claude-md-rules-list-incomplete.md`, both resolved 2026-08-29: a hand-kept list
of something the system already enumerates. Here the intent was even written
down first and then contradicted in the same file, which is what makes it worth
recording rather than quietly patching.

## Possible Solutions

- **Option A**: parse the entry point's output for the paths it reports and
  stage those. Uses the declared contract, removes the census, and a project
  adding a generator gets staging for free. Couples the hook to the exact
  wording of the output line — a contract worth stating explicitly in
  `docs-update.sh` if this is chosen.
- **Option B**: have `docs-update.sh` write the list of touched paths to a
  known location (stdout is already taken by human-readable progress; a
  `--porcelain` flag, or one path per line on fd 3). Same benefit, an explicit
  machine-readable contract instead of a parsed one, at the cost of a flag every
  project's entry point must implement.
- **Option C**: stage everything modified under `.development/` after the run.
  No census and no contract, but it would sweep unrelated edits the operator had
  in flight into a commit — silently, which is the failure mode being fixed.

## Recommended Approach

**Option A**, with the output format promoted to a documented part of the entry
point contract in the same change. Option B is cleaner in the abstract but asks
every project's `docs-update.sh` to grow a machine mode for one caller. Option C
trades a latent silent failure for an active one.

Whichever is chosen, `post-merge` inherits it automatically: it invokes this
module rather than carrying its own copy of the staging logic.

## Notes

- Split out of `tech-debt-index-generator-cannot-bootstrap.md` when that issue
  was resolved, so the finding outlives it.
- Distinct from the staging bug fixed alongside `post-merge-does-not-regenerate-derived-docs.md`,
  which was about *how* the module decided a file had changed (`git diff
  --quiet`, blind to untracked files). This one is about *which* files it looks
  at in the first place.

## Related Documentation

- **Architecture Decision**: `012-codebase-agnostic-automation.md` — hooks are
  generic orchestrators, stack knowledge lives in the entry points; this is a
  hook holding stack knowledge while claiming otherwise
- **Code Locations**:
  - `.githooks/pre-commit.d/04-docs-update` — the header claim and the
    `GENERATED` array
  - `.development/automation/docs-update.sh` — `run_generator()`, which prints
    the paths the array duplicates
  - `.githooks/post-merge` — the second caller, which inherits whatever this
    module does
