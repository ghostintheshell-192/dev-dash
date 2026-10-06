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
│   ├── idea-capture/
│   │   └── SKILL.md                # Capture / review / promote idea notes
│   └── session-handoff/
│       └── SKILL.md                # Session handoff skill
└── rules/                          # Auto-loaded project rules
    ├── overview.md                 # Project overview (template)
    ├── coding-standards.md         # Coding standards (template, language-specific)
    ├── principles.md               # Development principles
    ├── preflight-checks.md         # Pre-work checklist
    ├── idea-capture.md             # When to use the idea-capture skill
    └── workflow.md                 # Git workflow

.development/                       # Operational documentation
├── automation/                     # Standard entry points (ADR-012 pattern)
│   ├── bootstrap.sh                # Once-per-clone activation (hooksPath, exec bits)
│   ├── build.sh                    # Build the project (stack knowledge lives here)
│   ├── test.sh                     # Run tests
│   ├── format-check.sh             # Verify formatting
│   ├── format-fix.sh               # Apply formatting
│   └── docs-update.sh              # Regenerate derived docs
└── scripts/
    ├── generate-claude-config.sh   # Regenerates .claude/critical-rules.md from ADRs
    ├── generate-index.py           # INDEX.md generator
    ├── session-archive.py          # SessionEnd hook target
    ├── spec-workflow.py            # Spec lifecycle backend (planned → in-progress → implemented)
    └── update-tech-debt-index.py   # tech-debt/README.md generator

.githooks/                          # Generic orchestrator hooks (no stack commands)
├── pre-commit                      # Runs pre-commit.d/* in order
├── pre-commit.d/                   # 00-branch-protection, 01-security,
│                                   # 02-format-check, 03-archive-resolved-issues,
│                                   # 04-docs-update, 05-spec-workflow
├── post-checkout                   # Spec planned → in-progress on branch creation
└── post-merge                      # Spec → implemented on merge into develop

.memory-bank/                       # Session continuity
├── ideas/                          # Tangential idea notes (idea-capture skill)
└── sessions/                       # JSONL transcripts (auto-archived)

.devdash-default                    # Marks this scaffold as DevDash's default
                                    # (preselected in UI; never copied to projects)
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
6. **Run `bash .development/automation/bootstrap.sh`** once per clone to
   activate the git hooks (sets `core.hooksPath`, fixes exec bits). The
   entry points under `.development/automation/` ship with this scaffold's
   stack defaults — adapt their internals to the project's stack; hooks
   never need touching (ADR-012).

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
