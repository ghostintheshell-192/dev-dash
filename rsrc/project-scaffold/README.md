# Project Scaffold Template

Template structure that DevDash applies when initializing a new project with
Claude Code configuration.

## Structure

```text
.claude/                            # Claude Code project config
├── CLAUDE.md                       # Project entry point
├── settings.json                   # Hooks + permissions
├── commands/
│   └── handoff.md                  # /handoff slash command
├── skills/
│   └── session-handoff/
│       └── SKILL.md                # Session handoff skill
└── rules/                          # Auto-loaded project rules
    ├── overview.md                 # Project overview (template)
    ├── coding-standards.md         # Coding standards (template, language-specific)
    ├── principles.md               # Development principles
    ├── preflight-checks.md         # Pre-work checklist
    └── workflow.md                 # Git workflow

.development/                       # Operational documentation
└── scripts/
    └── session-archive.py          # SessionEnd hook target

.memory-bank/                       # Session continuity
├── ideas/                          # Tangential idea notes (idea-capture rule)
└── sessions/                       # JSONL transcripts (auto-archived)
```

## Usage

When initializing a project with DevDash:

1. **Copy scaffold structure** to project root
2. **Customize placeholders** in `.claude/CLAUDE.md` and `.claude/rules/overview.md`:
   - `{PROJECT_NAME}` — Project name
   - `{PROJECT_DESCRIPTION}` — Short description
   - `{TECH_STACK_DESCRIPTION}` — Tech stack details
   - `{PROJECT_STRUCTURE}` — Directory structure
   - `{LANGUAGE_SPECIFIC_STANDARDS}` — Coding standards for the detected language
3. **Initialize `.development/` skeleton** with `CURRENT-STATUS.md`, `ARCHITECTURE.md`,
   and the empty subfolders `specs/`, `tech-debt/`, `reference/decisions/`
4. **Create `.personal/`** for private notes (untracked)
5. **Create `docs/`** for public documentation

## Template Placeholders

DevDash replaces these placeholders during initialization:

- `{PROJECT_NAME}` — from project directory name or user input
- `{PROJECT_DESCRIPTION}` — from user input
- `{TECH_STACK_DESCRIPTION}` — auto-detected or user input
- `{PROJECT_STRUCTURE}` — auto-generated from directory scan
- `{LANGUAGE_SPECIFIC_STANDARDS}` — selected from language-specific templates

## Claude Code Pattern

This scaffold uses Claude Code's official `.claude/rules/` pattern: every `.md`
file in `.claude/rules/` is automatically loaded as project instruction. No
`@includes` are needed — Claude Code discovers them.

## Scope

This is a **project-level** scaffold. Each project owns its own
`.claude/` configuration and is fully self-contained. There is no
intermediate "workspace" layer.
