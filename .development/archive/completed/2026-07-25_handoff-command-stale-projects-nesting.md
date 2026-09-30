---
type: bug
priority: medium
status: resolved
discovered: 2026-07-24
resolved: 2026-07-25
related: []
related_decision: null
report: null
---

# `/handoff` command contradicts the session-handoff skill on where handoffs live

## Problem

`.claude/commands/handoff.md` tells Claude to write handoffs into
`.memory-bank/projects/<project-name>/`. The `session-handoff` skill
(`.claude/skills/session-handoff/SKILL.md`) and `.claude/rules/workflow.md`
both say the handoff lives flat in `.memory-bank/`.

The two files disagree, and the wrong one is the one shipped in the scaffold —
so every project that receives `rsrc/project-scaffold/` inherits the
contradiction and can grow a `projects/<name>/` subtree that nothing else in
the toolchain expects.

## Analysis

- `.claude/commands/handoff.md` and
  `rsrc/project-scaffold/.claude/commands/handoff.md` are **byte-identical**:
  the live config and the distributed template carry the same stale text.
- dev-dash itself writes handoffs flat (`.memory-bank/YYYY-MM-DD-HHmm-*.md`),
  so the command has simply never been followed here — the skill wins in
  practice, which is why the bug stayed invisible.
- Confirmed downstream: `raid-sandbox` has
  `.memory-bank/projects/raid-explorer/` with 12 handoffs. That project also
  has the identical `handoff.md`, so the structure is reproducible from the
  scaffold regardless of its original cause (that repo was split out of the
  personal-site repo, where a per-game subfolder did make sense).
- `.claude/rules/overview.md` only says "use `.memory-bank/` for continuity",
  so `handoff.md` is the single source of the wrong convention.
- Secondary: `handoff.md` starts with a UTF-8 BOM.

The generalizable point: an artifact that encodes a *superseded* convention
propagates silently through the scaffold. Same failure shape as shipping
generated `key-decisions.md` instead of its generator (ADR-013) — the scaffold
must carry conventions that are current, or it manufactures drift.

## Possible Solutions

- **Option A**: Fix `handoff.md` in both places to point at flat
  `.memory-bank/`, drop the "for each project" loop, strip the BOM. — Minimal,
  removes the contradiction at the source. Does not clean up projects already
  carrying `projects/<name>/`.
- **Option B**: Option A, plus make the command delegate to the skill instead
  of restating the procedure (`Invoke the session-handoff skill`). — Removes
  the possibility of the two drifting apart again; one place states the
  convention.
- **Option C**: Delete `commands/handoff.md` entirely and rely on the skill,
  which already triggers on farewell phrases and explicit requests. — Fewest
  moving parts, but loses the explicit `/handoff` affordance.

## Recommended Approach

Option B. The duplication is the actual defect: the command restating the
skill's procedure is what let the two versions diverge. Keeping `/handoff` as
a thin trigger preserves the affordance without a second copy of the rules.

Existing projects with `projects/<name>/` get flattened as part of their own
scaffold-alignment work, not from here.

## Notes

Found while auditing `raid-sandbox` against the scaffold (2026-07-24). That
audit also surfaced that `raid-sandbox` lacks the `## Session Start` section
of `.claude/rules/workflow.md` — the directive to *read* the latest handoff —
which is a per-project gap, not a scaffold bug: the scaffold's `workflow.md`
has it.

## Resolution (2026-07-25)

Option B applied to both copies, which remain byte-identical: `handoff.md` no
longer restates the procedure, it invokes the `session-handoff` skill and says
explicitly that the skill owns location, naming, and content structure. The
`/handoff` affordance survives as a thin trigger, and there is now only one
statement of the convention — the duplication that let the two drift is gone.

The UTF-8 BOM is stripped from both files (they opened with `EF BB BF`).

Verified: no reference to `.memory-bank/projects/` remains anywhere in the repo.

### Not covered

Projects that already grew the wrong structure are untouched, as this write-up
anticipated. `raid-sandbox` still has `.memory-bank/projects/raid-explorer/`
with 12 handoffs; flattening it belongs to that repo's own scaffold-alignment
work.

Also unaddressed, and noted here so it is not lost: `raid-sandbox` lacks the
`## Session Start` section of `.claude/rules/workflow.md` — the directive to
*read* the latest handoff. That is a per-project gap, not a scaffold defect; the
scaffold's `workflow.md` has it.

## Related Documentation

- **Code Locations**:
  - `.claude/commands/handoff.md` (line 8)
  - `rsrc/project-scaffold/.claude/commands/handoff.md` (line 8)
  - `.claude/skills/session-handoff/SKILL.md` (line 15)
  - `.claude/rules/workflow.md` ("Session End")
- **Architecture Decision**: ADR-013 (scaffold source of truth — the
  "distribute the generator, not the output" principle this generalizes)

---

📍 **Investigation Note**: Read [ARCHITECTURE.md](../ARCHITECTURE.md) to locate relevant files and understand the architectural context before starting your analysis.
