---
type: bug
priority: medium
status: resolved
discovered: 2026-08-29
resolved: 2026-08-29
related: []
related_decision: 012-codebase-agnostic-automation.md
report: null
---

# Derived docs are one regeneration behind after any merge of two independent branches

## Problem

`INDEX.md`, `tech-debt/README.md` and `ARCHITECTURE.md` are generated from the
tree and staged by `pre-commit.d/04-docs-update`. Pre-commit fires on
`git commit` — and git creates no-conflict merge commits *directly*, without
running it. So when two branches that each touched `.development/` meet on the
integration branch, nothing regenerates the derived docs against the merged
tree: they stay whatever the textual merge of the two sides produced.

Observed, not hypothetical. In `raid-sandbox`, merge `8b08978` brought together
a branch cut before three tech-debt issues landed and the branch that had added
them; the index that resulted listed neither. It took an explicit manual pass
(`34734be`, 2 files, +7/-1) to bring them back in line.

## Analysis

The mechanism is already documented in this repo — in the header of
`.githooks/post-merge`, which says outright that git creates no-conflict merge
commits *without* running pre-commit, and exists precisely to cover that gap.
It covers it for **specs** only: the hook calls `spec-workflow.py` and nothing
else. The identical gap for derived docs was never wired up, even though
`.development/automation/docs-update.sh` is exactly the entry point ADR-012
defines for the purpose and is already invoked by pre-commit.

Three separate holes, in increasing order of how much work they need:

**1. The hook does not call `docs-update.sh`.** Present in `dev-dash`,
`government-feed` and `synedria`; in all three it runs `spec-workflow.py` and
exits.

**2. Two repos have no `post-merge` at all.** `raid-sandbox` and `sheet-atlas`
ship only `pre-commit` (+ `pre-commit.d`, and `post-checkout` in `sheet-atlas`).
`raid-sandbox` is where the drift was actually caught, so fixing hole 1 alone
would leave the one repo with a confirmed instance still uncovered.

**3. The hook hardcodes `develop` as the integration branch**
(`[[ "$CURRENT_BRANCH" == "develop" ]] || exit 0`, line 28). `raid-sandbox`
integrates on `main`. Installing the hook there unchanged would produce a hook
that exits silently on every merge — worse than no hook, because it looks
covered.

`sheet-atlas` has no `.development/automation/docs-update.sh`, so it needs that
entry point before any of this applies to it. Out of scope here.

**Why medium and not high.** The staleness is bounded and self-healing: the next
commit that touches anything runs pre-commit, which regenerates from the full
merged tree and corrects it. The window is merge → next commit. That is a real
difference from `architecture-layer-overview-stale-prose` (resolved 2026-08-29,
in `archive/completed/`), where the wrong content was in a heredoc and no amount
of committing would ever have fixed it. Same family — auto-loaded documentation
drifting with nothing to make it visible — but this one has a floor.

It is not zero, either. `ARCHITECTURE.md` is `@include`-imported into every
session, so the window is exactly when a session started right after a merge
reads a map that does not describe the merged tree.

## Possible Solutions

- **Option A**: have `post-merge` call `docs-update.sh` after the spec step,
  stage the result, and reuse the existing amend-or-instruct ending. The hook
  already solves the "cannot amend while MERGE_HEAD exists" problem for specs
  and prints the `git commit --amend --no-edit` one-liner; the docs would ride
  the same path, and the operator's ritual after a merge stays one command.
- **Option B**: leave it to the operator, documented in `workflow.md` as a step
  to run after merging. Zero machinery. It is also what was in force until now,
  de facto, and `34734be` is the record of it failing.
- **Option C**: let `post-merge` create a *follow-up* commit rather than staging
  for an amend. Removes the manual step entirely, at the cost of a second commit
  after every merge that touched documentation.

Independently of the choice, hole 3 must be fixed for the hook to be portable:
accept a set of integration branches (`develop` *and* `main`) rather than the
single hardcoded name, in both `.githooks/post-merge` and the scaffold copy.

## Recommended Approach

**Option A**, plus the integration-branch generalisation, applied to
`rsrc/project-scaffold/.githooks/post-merge` first since ADR-013 makes the
scaffold the source of truth, then propagated to the repos that already carry
the hook, then installed in `raid-sandbox` (which needs hole 3 fixed to work at
all).

Option C is tempting — it is the only one with no manual step — but it splits
every documentation-touching merge into two commits, and the amend ritual is
already established for specs. Adding a second, different post-merge convention
to save one command is a bad trade.

## Solution Implemented

Resolved 2026-08-29 in `dev-dash` and `rsrc/project-scaffold/`. Option A, with
the integration-branch generalisation, and one deviation worth naming.

**The docs step invokes `pre-commit.d/04-docs-update` instead of
reimplementing it.** The obvious way to write Option A was to run
`docs-update.sh` from `post-merge` and stage the results — which would have
meant a second copy of the list of generated files, in a hook, next to the copy
already flagged in `tech-debt-index-generator-cannot-bootstrap.md`. That is the
pattern this repo keeps rediscovering, so: one module deciding what to
regenerate and what to stage, reached from two triggers. Three lines in
`post-merge`, and any future fix to the staging logic is inherited rather than
ported.

The hook was restructured so the two steps are independent. It used to bail out
early on a branch prefix it did not recognise or a spec it did not find, both
before any docs work could happen; the docs step now runs on every merge into an
integration branch. That matters immediately: `chore/` was never in the spec
prefix list, and a `chore/` branch is exactly the kind that archives an issue or
adds a note without carrying a spec.

**Hole 3 closed.** `INTEGRATION_BRANCHES=("develop" "main")` replaces the
hardcoded `develop`, so a repo integrating on `main` gets a working hook rather
than a silent one.

**A second defect, found while testing this one.** `04-docs-update` decided what
to stage with `git diff --quiet <file>`, which reports *no difference* for a
file git does not track yet. So whenever a generator **created** its output
instead of updating it, the new file was left untracked and the hook announced
that the derived docs were already current. Invisible in all five repos, where
the three files have long been tracked; fatal on a fresh project, which is
precisely when someone would be relying on the automation to work. It now adds
unconditionally — `git add` on an unchanged tracked file is a no-op, so the
pre-test bought nothing and got the create case wrong — and reports from the
index rather than the working tree.

## Testing

In a scratch repository built from these hooks and generators:

1. **Bootstrap** — first commit with no derived docs present: all three are
   created, staged and committed, working tree clean. Before the staging fix
   they were created and left untracked.
2. **Clean merge, stale docs** — a `chore/` branch adding a tech-debt issue
   committed with `--no-verify`, then merged with no conflict. This is the
   reported failure. `post-merge` regenerated, the index gained the entry, and
   both files were staged for the amend.
3. **Concurrent branches** — two branches each adding an issue. Result below.
4. **Spec path unaffected** — a merge with nothing to stage exits silently
   instead of telling the operator to amend an unchanged commit.

## Impact

Closes the window between a merge and the next commit during which
`ARCHITECTURE.md` — `@include`-imported into every session — could describe a
tree that no longer existed.

## Follow-up, not fixed here

Test 3 surfaced something this note did not anticipate: when two branches each
regenerate the same derived file, git **conflicts** on it. The conflict is not
dangerous — resolving to either side and committing runs pre-commit, which
regenerates correctly, and the final index was verified to contain all three
issues with a clean tree. But the operator is resolving a conflict in a file
that is about to be overwritten wholesale, which is busywork.

The fix is a merge driver: a `.gitattributes` marking the three derived files
`merge=generated`, and `bootstrap.sh` registering
`git config merge.generated.driver true` (keep either side; `post-merge`
regenerates). It is left out of this change deliberately — it adds a per-clone
config step alongside `core.hooksPath`, which is a setup decision rather than a
hook fix.

Propagation to `government-feed`, `synedria` (which have the old hook) and
`raid-sandbox` (which has none, and needed hole 3 fixed to work at all) is also
still open. `sheet-atlas` needs `docs-update.sh` first.

## Notes

- Promised in the body of `raid-sandbox` `34734be`: *"A post-merge hook calling
  docs-update.sh would close it; filed separately rather than smuggled into this
  commit."* This is that filing.
- The drift only became visible because `INDEX.md` and `tech-debt/README.md`
  were brought back under version control the same day (`raid-sandbox`
  `1e55217` reverted the gitignore exclusion once the generators stopped
  stamping a timestamp). While they were untracked, this failure mode existed
  and could not be observed at all.
- Fixing this makes the derived docs verifiable the same way they are now in
  `dev-dash`: regenerate, and a clean working tree proves the committed state is
  the deterministic output.

## Related Documentation

- **Architecture Decision**: `012-codebase-agnostic-automation.md` — hooks are
  generic orchestrators, stack knowledge lives in `.development/automation/`
  entry points; `docs-update.sh` is the entry point this hook should be calling.
  Also `013-scaffold-source-of-truth.md` for the propagation order.
- **Code Locations**:
  - `.githooks/post-merge` — the spec-only body, and the `develop` hardcode at
    line 28
  - `.githooks/pre-commit.d/04-docs-update` — what runs on `git commit` and not
    on merges
  - `.development/automation/docs-update.sh` — the entry point to call
  - `rsrc/project-scaffold/.githooks/post-merge` — the copy every new project
    inherits
- **Prior instance**: `raid-sandbox` `8b08978` (the merge that drifted),
  `34734be` (the manual correction)
