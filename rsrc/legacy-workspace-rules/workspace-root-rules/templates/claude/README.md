# Claude Code Project Templates

Templates for self-contained Claude Code configuration in projects.

## Quick Start

Initialize a new project with Claude Code configuration:

```bash
cd /path/to/new-project
python3 /data/repos/.rules/templates/setup-claude-config.py .
```

This creates:
- `.claude/` - Configuration, commands, skills
- `.development/scripts/` - Session archive script
- `.memory-bank/` - Handoffs and session transcripts

## What Gets Created

### `.claude/`

**settings.json**:
- Pre-approved bash commands (dotnet, grep, find, etc.)
- SessionEnd hook for auto-archiving transcripts

**commands/handoff.md**:
- `/handoff` command for session summaries

**skills/session-handoff/**:
- Auto-creates handoff notes when you say goodbye

### `.development/scripts/`

**session-archive.py**:
- Archives session transcripts to `.memory-bank/sessions/`
- Called automatically on session end

### `.memory-bank/`

**Handoff files** (YYYY-MM-DD-HHmm-title.md):
- Session summaries created by handoff skill
- One file per session

**sessions/**:
- Auto-archived session transcripts (.jsonl)
- Gitignored by default

## Template Structure

```
.rules/templates/claude/
├── settings.json          # Permissions + hooks
├── commands/
│   └── handoff.md         # Handoff command
└── skills/
    └── session-handoff/   # Auto-handoff skill
        └── SKILL.md
```

## Updating Templates

To update templates for new projects:

1. Edit files in `.rules/templates/claude/`
2. Run setup script on new projects
3. Existing projects: copy manually or re-run script (merges settings)

## Self-Contained Projects

Each project is fully portable:
- Copy project folder → Claude config works
- No dependency on workspace or global config
- Perfect for external drives or sharing

## See Also

- `setup-project-docs.py` - Setup .personal/ folder
- `.rules/core/principles.md` - Development principles
- `.rules/workflows/git.md` - Git workflow
