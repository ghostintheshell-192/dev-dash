# Session Workflow

What to do at the start, during, and end of each coding session.

---

## Session Start

1. **Hook auto-runs** `generate-index.py` (if configured)

2. **Read project context**:
   - `INDEX.md` → recent changes, file structure
   - `CURRENT-STATUS.md` → where we left off

3. **Check session notes**:
   - Location: `.memory-bank/projects/[project-name].md`
   - Contains: last session summary, decisions made, next steps

4. **Pre-flight checks** (for coding projects):
   - Git status clean?
   - On correct branch?
   - Run `goto.yaml` workflow for language detection

---

## During Session

### New information goes to the right place

| Type | Destination |
|------|-------------|
| New ideas | `active/current-notes.md` or `ideas/` |
| Important decisions | `reference/decisions/` (as ADR) |
| Bugs/tech-debt found | `active/tech-debt/` (use `_TEMPLATE.md`) |
| Session learnings | Working notes, then consolidate |

### When to update documentation

- **Immediately**: Critical decisions, gotchas discovered
- **Before context switch**: Consolidate scattered notes
- **Before ending session**: Prepare handoff for next session

---

## Session End

### Update session notes

Location: `.memory-bank/projects/[project-name].md`

Format (new entries at top):

```markdown
## YYYY-MM-DD - Brief Title

**Done**:
- What was accomplished
- Files changed
- Decisions made

**Next**:
- Next steps
- Blockers identified

**Notes**:
- Context for future sessions
- Gotchas to remember
```

### Cleanup

- Move scattered notes from `active/` to proper location
- Archive completed issues
- Update `CURRENT-STATUS.md` if significant progress

---

## Handoff Principle

Every session should end with enough context that a **different Claude instance** could continue the work seamlessly.

The handoff file (`.memory-bank/projects/*.md`) is your external memory — use it.

---

## Session Notes vs Conversation Archive

| Purpose | Location | Format |
|---------|----------|--------|
| **Handoff** (operational) | `.memory-bank/projects/` | Structured markdown |
| **Archive** (historical) | `.memory-bank/sessions/` | Full transcripts |

The hook `session_end_archive.py` automatically saves transcripts. Session notes are **your responsibility** to update.

---

*For .personal/ folder structure, see `workflows/personal-folder.md`*
*For git workflow, see `workflows/git.md`*
