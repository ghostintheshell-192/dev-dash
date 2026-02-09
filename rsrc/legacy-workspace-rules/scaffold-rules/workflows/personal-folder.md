# .personal/ Folder Structure

Private documentation for spec-driven development. NOT committed to git.

---

## Purpose

Every project should have a `.personal/` folder for:

- Planning and roadmaps
- Working notes and ideas
- Business/marketing documentation
- Tech debt tracking
- Architecture Decision Records

---

## Standard Structure

```text
.personal/
├── INDEX.md                 # Auto-generated navigation (DO NOT EDIT)
├── CURRENT-STATUS.md        # Current project state (1 page max)
├── README.md                # Folder documentation
│
├── planning/                # Strategic planning
│   └── roadmap-*.md        # Roadmaps and plans
│
├── business/                # Business & marketing
│   └── marketing/          # Marketing strategy
│
├── active/                  # Current work
│   ├── tech-debt/          # Tech debt tracking (individual files)
│   │   ├── _TEMPLATE.md    # Template for new issues
│   │   ├── README.md       # Tech debt workflow documentation
│   │   └── issue-name.md   # Individual tracked issues
│   ├── current-notes.md    # Working notes
│   └── issue_*.md          # Active issues
│
├── reference/               # Reference documentation
│   ├── technical/          # Technical notes
│   ├── decisions/          # Architecture Decision Records
│   └── checklists/         # Workflow checklists
│
├── specs/                   # Feature specifications
│   ├── implemented/        # Completed features
│   ├── in-progress/        # Currently being built
│   ├── planned/            # Next up
│   └── backlog/            # Future ideas
│
├── archive/                 # Historical
│   ├── completed/          # Finished projects
│   ├── analysis/           # Past analyses, agent reports
│   └── postmortems/        # Post-mortems
│
├── ideas/                   # Future ideas & PoCs
│
└── scripts/                 # Utility scripts
    └── generate-index.py   # Index generator
```

---

## Auto-Generated INDEX.md

Each project with `.personal/` should have a `generate-index.py` script that:

- Scans `.personal/` and `docs/` folders
- Generates `INDEX.md` with file listings
- Shows file sizes and modification dates
- Marks recent files (last 7 days)

**Setup**: Copy template from `.rules/templates/.personal/scripts/generate-index.py`

---

## SessionStart Hook

Projects can auto-generate the index at session start via `.claude/settings.json`:

```json
{
  "hooks": {
    "SessionStart": [
      {
        "matcher": "startup",
        "hooks": [
          {
            "type": "command",
            "command": "python3 /path/to/project/.personal/scripts/generate-index.py 2>/dev/null || true"
          }
        ]
      }
    ]
  }
}
```

---

## Quick Setup for New Projects

Run the setup script to initialize documentation structure:

```bash
python3 .rules/templates/setup-project-docs.py /path/to/project
```

This creates:

- `.personal/` folder structure
- `generate-index.py` script
- `.claude/settings.json` with SessionStart hook
- Template files (README.md, CURRENT-STATUS.md)

---

## Key Principles

### One source of truth per topic

| Topic | Location |
|-------|----------|
| Planning | `planning/` |
| Current work | `active/` |
| Reference | `reference/` |
| Done | `archive/` |
| Specifications | `specs/` |

### Don't duplicate

If something exists, **update it** instead of creating a new file.

---

*For session workflow (when to read/update these files), see `workflows/session.md`*
