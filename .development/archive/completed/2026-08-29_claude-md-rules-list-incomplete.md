---
type: code-quality
priority: low
status: resolved
discovered: 2026-07-05
related: [architecture-layer-overview-stale-prose.md]
related_decision: null
report: null
---

# .claude/CLAUDE.md's rule enumeration omits idea-capture.md

## Problem

`.claude/CLAUDE.md` documents which files are loaded from `.claude/rules/`:

> overview.md, coding-standards.md, principles.md, preflight-checks.md,
> workflow.md

The directory actually contains six files — `idea-capture.md` exists and is
loaded (Claude Code auto-loads every `.md` under `.claude/rules/`, per the
file's own opening paragraph) but isn't listed.

## Analysis

Low-stakes instance of the same pattern as the ARCHITECTURE.md Layer Overview
issue: a hand-written enumeration describing something that's mechanically
auto-loaded, with no check tying the two together. `idea-capture.md` was
presumably added after this list was last edited. Functionally harmless — the
rule still loads regardless of whether it's mentioned — but it means the
"Project Rules" section of CLAUDE.md can no longer be trusted as a complete
index without cross-checking the directory.

## Possible Solutions

- **Option A — generate the list**: have a script (or a small addition to an
  existing hook) enumerate `.claude/rules/*.md` and rewrite that section of
  `CLAUDE.md`, mirroring how `ARCHITECTURE.md`'s Project Tree is generated.
  Removes the drift class entirely, at the cost of one more generated section
  to keep straight from hand-written ones (see the Layer Overview issue for
  what happens when that boundary gets blurry).
- **Option B — one-line manual fix**: just add `idea-capture.md` to the list
  now. Zero cost, but the same drift will recur the next time a rule file is
  added or removed.
- **Option C — replace the itemized list with a pointer**: change the
  sentence to "all `.md` files under `.claude/rules/` are loaded
  automatically — see that directory for the current set" and drop the
  enumeration. Nothing to keep in sync because nothing is duplicated.

## Recommended Approach

**Option C.** The itemized list adds little value over just looking at the
directory, and every other instance of this failure pattern in this audit
came from a duplicated enumeration that nobody re-checks. Removing the
duplication is more durable than committing to keep it updated.

## Solution Implemented

Resolved 2026-08-29. Option C: the enumeration is gone from `.claude/CLAUDE.md`,
and from `rsrc/project-scaffold/.claude/CLAUDE.md` so new projects do not inherit
it (ADR-013). What remains is one sentence stating that every `.md` under
`.claude/rules/` is loaded automatically.

The recommendation above held, and one argument it did not make turned out to be
decisive: those six rule files are loaded **in full** into every session. The
list was not a summary of something absent — it was a second, shorter copy of
something already entirely in context. It could only ever subtract, by drifting.

Deliberately, the file does not explain what was removed or why. `.claude/CLAUDE.md`
is read at the start of every session, so prose justifying an absence would be
paid for in every one of them, forever. The reasoning belongs here and in the
commit; the file keeps only what a session needs to act.

## Testing

`ls .claude/rules/` returns six files, all still loaded, none of them named
anywhere in `CLAUDE.md`. There is nothing left to fall out of sync.

## Impact

Removes one of the three hand-kept censuses found in this audit. The other two
were `architecture-layer-overview-stale-prose.md` (resolved the same day) and
`.githooks/pre-commit.d/04-docs-update`'s hardcoded `GENERATED` array, recorded
in `tech-debt-index-generator-cannot-bootstrap.md` and still open.

## Notes

- Found in the same pass as `architecture-layer-overview-stale-prose.md`.
  Both are enumerations-of-auto-loaded-content that drifted; this one is
  cosmetic, that one is materially misleading — worth fixing both for the
  pattern, not just the individual instances.

## Related Documentation

- **Code Locations**: `.claude/CLAUDE.md` — "Project Rules" section;
  `.claude/rules/` — six files, five listed

---

📍 **Investigation Note**: Read [ARCHITECTURE.md](../ARCHITECTURE.md) to
locate relevant files and understand the architectural context before
starting your analysis.
