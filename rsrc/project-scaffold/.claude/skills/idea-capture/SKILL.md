---
name: idea-capture
description: Use to park a tangential idea without derailing the current work, to review the parked ideas, or to promote one into a spec, tech-debt entry, ADR or skill. Activate when the user says "note it and move on", "save it for later", "park it", "out of scope", "take a note", asks what ideas are open, or says the same in their native language; also when a sub-agent's report carries an "Ideas" section.
allowed-tools: Read, Write, Edit, Glob, Grep, Bash
---

# Idea Capture

Ideas live in `.memory-bank/ideas/`, one file each. This skill has three modes:
**capture** (write one), **review** (look at what is parked), **promote** (turn one
into a durable artifact). Pick the mode from the request; capture is the default.

An idea note is not a spec. It records that a thought existed, where it came from,
and the smallest step that would pick it up again. Keep it short.

## Who writes the notes

Only the **main session** writes idea notes. Sub-agents do not:

- many of them (`Explore`, `Plan`) cannot write files at all;
- they cannot ask the user, and the filter is the user's judgement;
- an agent told to "note ideas" while reading code produces many, mostly noise.

A sub-agent lists what it noticed under an `Ideas` heading in its final report
(see the Agent Delegation Protocol in `.claude/CLAUDE.md`). When a report comes
back with one, the main session reads it, discards what is obvious or already
captured, and offers the rest to the user before capturing anything.

## Mode 1 — Capture

1. **Check for a duplicate.** Grep `.memory-bank/ideas/` for the key terms. If a
   note already covers it, add a dated line to that note instead of a new file.

2. **Create the file**: `.memory-bank/ideas/YYYY-MM-DD-<short-slug>.md` (today's
   date, lowercase hyphenated slug, max ~50 chars).

3. **Frontmatter** (required):

   ```yaml
   ---
   captured: YYYY-MM-DD
   status: open
   context: "where the idea emerged (workstream, branch, file)"
   tags: [tag1, tag2]
   ---
   ```

   If the idea came from a sub-agent's report, say so in `context`
   (e.g. `"Explore agent, reading src/engine/layout.js on feature/x"`).

4. **Body**: three short parts, no more.
   - **What** the idea is
   - **Why** it deserves future attention
   - **Next step**: the minimal action if it is ever picked up

5. **Resume the main thread.** Confirm in one line (the filename) and go back to
   what you were doing. Capture is not a detour.

### Proactive capture

When you sense a tangent that has its own merit but would derail the current work,
**ask** "want me to note it as an idea to explore?" instead of following it. Do not
capture proactively without asking.

## Mode 2 — Review

Use when the user asks what is parked, or at the start of planning work.

1. Read the frontmatter of every note in `.memory-bank/ideas/` (skip `README.md`).
2. Group by `status`. Leave `promoted-to-*` and `dropped` out of the summary,
   except to report a `promoted_to` link that no longer resolves.
3. For each `open` / `parked` note, give one line: date, slug, the *What* in a
   few words, and a suggestion — **keep**, **park**, **promote** (to what), or
   **drop** — with the reason. Flag `open` notes older than ~30 days: they have
   either matured or gone stale.
4. **Do not change any status yourself.** The review is a proposal; the user
   decides, then apply the decisions (Mode 3 for promotions, a frontmatter edit
   for the rest).

## Mode 3 — Promote

When an idea has matured into something durable:

1. Create the new artifact in its natural location (e.g.
   `.development/specs/planned/feature-x.md`, a `tech-debt/` entry, an ADR in
   `.development/reference/decisions/`, a skill in `.claude/skills/`).
2. **Update the idea note — never delete it.** Edit its frontmatter:

   ```yaml
   status: promoted-to-spec
   promoted_to: ../../.development/specs/planned/feature-x.md
   promoted_at: YYYY-MM-DD
   ```

3. From that moment the new artifact is the ground truth; the note stays as a
   record of where it came from.

To drop or park an idea, set `status: dropped` / `status: parked` and add one line
to the body saying why. The note is kept either way.

## Allowed states

- `open` — fresh capture
- `parked` — explicitly deferred
- `promoted-to-spec` / `promoted-to-tech-debt` / `promoted-to-adr` /
  `promoted-to-skill` — promoted; link in `promoted_to`
- `dropped` — decided as not actionable
