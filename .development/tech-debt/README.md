# Tech Debt Issues

This folder contains individual technical debt issues for DevDash.

## Structure

Each issue is a separate markdown file with standardized frontmatter:

```yaml
---
type: [bug|feature|refactor|performance|testing|code-quality|security]
priority: [high|medium|low]
status: [open|in-progress|resolved|closed|rejected]
discovered: YYYY-MM-DD
resolved: null  # YYYY-MM-DD when status leaves open/in-progress
related: []  # List of related issue filenames
related_decision: null  # Optional: link to reference/decisions/NNN-name.md
report: null  # Optional: link to archive/analysis/YYYY-MM-DD_report_agent-name.md
upstream: null  # Optional: the dependency the debt lives in
upstream_link: null  # Optional: the issue or PR opened upstream
---
```

### Upstream debt

Debt that lives in a dependency, not in DevDash (a limit of ImGuiDot, a gap in
the tree-sitter grammar), carries `upstream:` with the dependency's name. The
index lists it in a section of its own, grouped by dependency, so that it is
in one place when the time comes to report it there. `upstream_link:` records
the issue or PR once opened: without it the issue reads "not reported yet".
It is resolved when the fix is merged upstream and DevDash uses it (pin
updated, workaround removed).

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
4. Set `resolved:` to the date. The archiving step reads it to name the file,
   so an issue archived late still carries the date it was actually closed.
   Left null, the file is prefixed with the day it happened to be moved and
   the tools say so.

### Archiving Completed Issues

**Automatic** (recommended):

1. Change status to `resolved`, `closed`, or `rejected` in frontmatter
2. Commit — `03-archive-resolved-issues` handles staged issues — or run
   `../scripts/archive-resolved-issues.sh` for a whole-folder sweep
3. The file moves to `archive/completed/`, prefixed with `resolved:` (or
   `closed:`) from its frontmatter

**Manual**:

1. Add date prefix: `YYYY-MM-DD_issue-name.md` — the closure date, not today's
2. Move to `../archive/completed/`
3. Delete from `tech-debt/`

## Current Issues by Priority

**High Priority:** None currently

**Medium Priority:**
- `line-level-promote.md` - Promote e Apply operano solo sull'intero file
- `non-markdown-files-rendered-as-markdown.md` - File non-Markdown renderizzati come Markdown (script, config)

**Low Priority:**
- `docs-update-orchestrator-hardcodes-generated-files.md` - `04-docs-update` declares it knows nothing about the generators, then hardcodes their outputs
- `markdown-code-block-styling.md` - Fenced code blocks rendered as flat yellow text — no syntax highlighting
- `preprocess-imports-indented-fences.md` - PreprocessImports does not recognise indented fenced code blocks
- `scanner-directory-include-silent.md` - ConfigFileScanner: @include verso directory accettato e poi fallisce in silenzio

### Upstream — to report to the dependencies

**ImGuiDot:**
- `imguidot-box-size-font-metrics.md` - ImGuiDot: node boxes far larger than their text (not reported yet)
- `imguidot-diagram-size-api.md` - ImGuiDot: no API for the size of a diagram (not reported yet)
- `imguidot-fillcolor-without-filled.md` - ImGuiDot: `fillcolor` applied without `style=filled` (not reported yet)
- `imguidot-line-styles-ignored.md` - ImGuiDot: `style=dashed` and `style=dotted` ignored (not reported yet)
- `imguidot-record-shape.md` - ImGuiDot: `shape=record` not drawn (not reported yet)
- `imguidot-reserved-space-border.md` - ImGuiDot: reserved space cuts the outer borders (not reported yet)
- `imguidot-style-leftovers.md` - ImGuiDot: two leftovers of the style colours (#19) (not reported yet)

**tree-sitter-cpp:**
- `tree-sitter-cpp-default-argument-braces.md` - tree-sitter-cpp: `= {}` default argument parsed as an error (not reported yet)

## Integration with Reference Documentation

### Linking to Architecture Decisions

If an issue relates to an architectural decision:

```yaml
---
related_decision: 001-stack-tecnologico.md
---
```

This helps understand context: "Why was this pattern chosen? What were the trade-offs?"

### Agent-Generated Issues

When agents (code-reviewer, security-auditor, etc.) find issues:

- Issue created automatically in `tech-debt/`
- Full report in `archive/analysis/YYYY-MM-DD_report_agent-name.md`
- Issue links to report via `report:` field

### Creating Architecture Decisions

If resolving an issue requires a significant architectural choice:

1. Document decision in `../reference/decisions/NNN-name.md`
2. Link from issue: `related_decision: NNN-name.md`
3. Update `.claude/key-decisions.md` if Impact ≥ high (auto-generated)

## Tips

- Use descriptive slugs for filenames
- Keep frontmatter up to date
- Link related issues in `related` field
- Link to architectural decisions in `related_decision` if applicable
- Date prefix ONLY when moving to archive (automatically handled by script)
- Check `_TEMPLATE.md` for structure
