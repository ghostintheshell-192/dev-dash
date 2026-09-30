---
captured: 2026-07-25
status: open
context: "scaffold consolidation on dev-dash; relocated from .development/tech-debt/session-handoff-skill-note.md, where it sat as an untitled working note with no frontmatter"
tags: [scaffold, session-handoff, claude-config]
---

# Does the session-handoff skill need its own localized-triggers section?

## What

The original note asked whether the passage in
`rsrc/project-scaffold/.claude/skills/session-handoff/SKILL.md` about
recognising exit phrases in the user's own language is actually necessary.

Looking at it during the scaffold audit, the question has a concrete edge: the
same instruction exists in two places, at two different granularities.

- `SKILL.md` closes with a **Localized triggers** section, phrased generically:
  *"Recognize these phrases in whatever language the user speaks"*, with
  English examples. The skill's own frontmatter `description` already says
  *"Also activate when the user says goodbye or requests a handoff/summary in
  their native language."*
- dev-dash's `.claude/rules/workflow.md` (Session End) separately enumerates
  the **Italian** phrases: *"fermiamoci", "è tardi", "chiudiamo", "continuiamo
  domani", "ciao", "/exit", "/clear"*. The scaffold's `workflow.md` does not
  carry that list.

So the generic statement is made twice inside SKILL.md (description plus
section), and a project-specific instance lives in a third file.

## Why it deserves future attention

It is the same shape as two defects already fixed in this consolidation pass:
`handoff-command-stale-projects-nesting` (the `/handoff` command restating the
skill's procedure until the two disagreed) and
`claude-md-rules-list-incomplete` (a hand-maintained enumeration of
auto-loaded content). A convention stated in more than one place drifts; the
durable fix has consistently been to remove the duplication rather than commit
to keeping the copies in sync.

Low stakes: nothing here is wrong today, and both statements point the same
way. It is a tidiness question, not a correctness one.

## Minimal next step

Decide which layer owns the trigger vocabulary:

- **Skill only** — drop the standalone Localized triggers section, since the
  frontmatter `description` already carries the instruction; keep any
  language-specific list out of the scaffold, where it cannot be right for
  every project.
- **Or skill generic + project-specific list in `workflow.md`** — the current
  arrangement, made deliberate: the skill states the principle, each project
  lists the phrases its user actually says. If this one, say so in SKILL.md so
  the duplication reads as intentional.

The second is defensible; the point is that right now neither was chosen.
