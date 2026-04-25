# .development/ - Development Documentation

Private development documentation for spec-driven development workflow.

**Committed to git** - shared development documentation.

## Quick Start

1. Read `CURRENT-STATUS.md` for project state
2. Read `INDEX.md` for full navigation (auto-generated)
3. Check `tech-debt/` for active issues

## Structure

```text
.development/
├── CURRENT-STATUS.md      # Current project state
├── INDEX.md               # Auto-generated navigation
├── ARCHITECTURE.md        # Auto-generated project tree
│
├── specs/                 # Feature specifications
│   ├── implemented/       # Completed features
│   ├── in-progress/       # Currently being developed
│   ├── planned/           # Confirmed for next releases
│   ├── backlog/           # Validated but not scheduled
│   └── archived/          # Deprecated/cancelled
│
├── tech-debt/             # Active technical debt
│   ├── _TEMPLATE.md       # Template for new issues
│   └── issue-name.md      # Individual tracked issues
│
├── reference/             # Reference documentation
│   ├── decisions/         # Architecture Decision Records (ADR)
│   ├── technical/         # Technical notes
│   └── checklists/        # Workflow checklists
│
├── archive/               # Historical
│   ├── completed/         # Resolved issues
│   ├── analysis/          # Agent reports
│   ├── postmortems/       # Post-mortems
│   └── legacy/            # Old files
│
└── scripts/               # Utility scripts
    ├── generate-index.py
    ├── update-tech-debt-index.py
    ├── archive-resolved-issues.sh
    ├── spec-workflow.py
    ├── generate-architecture.sh
    ├── generate-claude-config.sh
    ├── extract-summary.sh
    └── session-archive.py
```

## Workflow

### Tech Debt

```text
1. Create: tech-debt/issue-name.md (use _TEMPLATE.md)
2. Work: Update notes, track progress
3. Resolve: Change status to "resolved"
4. Archive: Run scripts/archive-resolved-issues.sh
```

### Architecture Decisions (ADR)

Create in `reference/decisions/` when:

- Architectural pattern choice
- Technology selection
- Cross-cutting concern decision
- Trade-off impacting multiple components

Format: `NNN-descriptive-name.md`

### Spec Lifecycle (Automated)

Git hooks move specs between status directories on branch/merge events:

- `git checkout -b feature/<name>` → spec moves to `in-progress/`
- Merge to `develop` → spec moves to `implemented/`

Backend: `scripts/spec-workflow.py`. Activation: `git config core.hooksPath .githooks`.

### Agent Reports

Agents write to:

- Full report → `archive/analysis/YYYY-MM-DD_report_agent-name.md`
- Critical findings → `tech-debt/` as individual issues

## Related

- Public docs: `docs/`
- Personal notes: `.personal/`

---

*Run `python scripts/generate-index.py` to regenerate INDEX.md*
