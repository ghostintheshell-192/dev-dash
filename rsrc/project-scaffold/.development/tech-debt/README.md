# Tech Debt Issues

This folder contains individual technical debt issues for {PROJECT_NAME}.

## Structure

Each issue is a separate markdown file with standardized frontmatter:

```yaml
---
type: [bug|feature|refactor|performance|testing|code-quality|security]
priority: [high|medium|low]
status: [open|in-progress|resolved|closed|rejected]
discovered: YYYY-MM-DD
related: []  # List of related issue filenames
related_decision: null  # Optional: link to reference/decisions/NNN-name.md
report: null  # Optional: link to archive/analysis/YYYY-MM-DD_report_agent-name.md
---
```

The frontmatter is not decoration: `update-tech-debt-index.py` reads `priority`
and `status` from it to build the index below, and the
`03-archive-resolved-issues` pre-commit hook reads `status` to decide what to
archive. An issue file without frontmatter is invisible to both — it will not
appear in the index and it will never auto-archive. Always start from
`_TEMPLATE.md`.

## Workflow

### Creating New Issues

1. Copy `_TEMPLATE.md`
2. Rename to descriptive slug: `issue-name.md` (NO DATE PREFIX)
3. Fill in frontmatter and content
4. Status starts as `open`

### Working on Issues

1. Update status to `in-progress`
2. Work on fix/implementation
3. When complete, add resolution sections:
   - Solution Implemented
   - Testing
   - Impact

### Archiving Completed Issues

**Automatic**: set `status` to `resolved`, `closed`, or `rejected` in the
frontmatter and commit. The `03-archive-resolved-issues` pre-commit hook moves
the file to `../archive/completed/` with a date prefix and stages both sides of
the move.

**Manual** (if hooks are not active):

1. Add date prefix: `YYYY-MM-DD_issue-name.md`
2. Move to `../archive/completed/`

## Current Issues by Priority

*Auto-updated: 2026-08-29 18:44*

**High Priority:** None currently

**Medium Priority:** None currently

**Low Priority:** None currently

## Integration with Reference Documentation

### Linking to Architecture Decisions

If an issue relates to an architectural decision:

```yaml
---
related_decision: NNN-name.md
---
```

This helps understand context: "Why was this pattern chosen? What were the
trade-offs?"

### Agent-Generated Issues

When agents (code-reviewer, security-auditor, etc.) find issues:

- Issue created in `tech-debt/`
- Full report in `../archive/analysis/YYYY-MM-DD_report_agent-name.md`
- Issue links to report via the `report:` field

### Creating Architecture Decisions

If resolving an issue requires a significant architectural choice:

1. Document decision in `../reference/decisions/NNN-name.md`
2. Link from issue: `related_decision: NNN-name.md`
3. Update `.claude/key-decisions.md` if Impact ≥ high (auto-generated)

## Tips

- Use descriptive slugs for filenames
- Keep frontmatter up to date
- Link related issues in the `related` field
- Date prefix ONLY when moving to archive (handled by the hook)
- Check `_TEMPLATE.md` for structure
