---
captured: 2026-08-29
status: parked
context: "develop, after closing architecture-layer-overview-stale-prose and claude-md-rules-list-incomplete and opening two notes on the derived-docs pipeline"
tags: [configuration, automation, documentation, audit]
---

# Re-study the repository as a whole, and make its configuration discoverable at a glance

## What this is

A deliberate pass over the entire repository — not to fix anything, but to
establish what its configuration actually consists of: every hook, every
generator, every auto-loaded file, every entry point, and how they call each
other. The output is something that can be re-read in one look at the start of
any session, instead of being rediscovered piecemeal.

## Why it deserves attention

Valentina's diagnosis, and the reason this is not just tidying: *"non dominiamo
la configurazione nel suo complesso, non ne conosciamo tutti i pezzi e li
riscopriamo insieme ogni volta"*. It is the failure mode of a project that began
with one idea and grew with the work.

The session that prompted it is the evidence. Four defects, all the same shape —
**a hand-kept list of something the system can already enumerate**:

| Instance | The list | What already knew |
| --- | --- | --- |
| `architecture-layer-overview-stale-prose` (resolved) | classes per layer, in a heredoc | the Project Tree, generated 40 lines below |
| `claude-md-rules-list-incomplete` (resolved) | the files in `.claude/rules/` | Claude Code, which loads all of them in full |
| `post-merge-does-not-regenerate-derived-docs` (open) | which automations run after a merge — specs, not docs | `docs-update.sh`, the declared entry point |
| `04-docs-update`'s `GENERATED` array (open, in `tech-debt-index-generator-cannot-bootstrap`) | the three files to stage | the entry point, which prints what it touched |

The last is the sharpest: that hook's header claims it has *"no knowledge of
generators or stack"*, directly above the hardcoded census that gives it exactly
that knowledge. The intent was written down and then contradicted four lines
later, and it stayed that way because nobody was reading the file as part of a
whole.

None of these was hard to fix once seen. Each cost far more to *find*, and each
was found by accident while doing something else.

## Minimal next step

Enumerate before designing anything. One pass listing what exists — `.githooks/`
and every `pre-commit.d/` module, `.development/scripts/`,
`.development/automation/`, everything auto-loaded via `.claude/`, and the
`@include` graph between them — and for each, what triggers it and what it
writes. The gaps should fall out of that table rather than being hunted for.

Only then decide the durable form: whether it is a generated map, a new
`.development/` document, or a change to what `ARCHITECTURE.md` means. That last
option connects to a decision parked earlier the same day — whether
`ARCHITECTURE.md` should become a map of the whole repository, with a semantic
change and a rename, absorbing `INDEX.md`.

Whatever the form, it has to be **derived**, not written. A hand-kept map of the
configuration would be the fifth instance of the table above.
