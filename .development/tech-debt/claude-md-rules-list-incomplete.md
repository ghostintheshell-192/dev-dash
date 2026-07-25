---
type: code-quality
priority: low
status: open
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
