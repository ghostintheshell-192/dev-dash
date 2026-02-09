# {PROJECT_NAME} - Claude Code Project Configuration

This project uses Claude Code's official `.claude/rules/` pattern. All `.md` files in the `rules/` directory are automatically loaded as project instructions.

## Project Overview

{PROJECT_DESCRIPTION}

## Project Rules

The following rules are automatically loaded from `.claude/rules/`:
- **overview.md** - Project overview, development methodology, and tech stack
- **coding-standards.md** - Coding standards and conventions
- **principles.md** - General development principles
- **preflight-checks.md** - Pre-flight checks before coding
- **workflow.md** - Git workflow and development commands

## Documentation Structure

- **[.development/](.development/)** - Operational documentation
  - **[ARCHITECTURE.md](.development/ARCHITECTURE.md)** - Project tree
  - **[CURRENT-STATUS.md](.development/CURRENT-STATUS.md)** - Project state
  - `specs/` - Feature specifications
  - `tech-debt/` - Known issues
  - `reference/decisions/` - ADRs
- **[docs/](docs/)** - Public documentation
- **[.personal/](.personal/)** - Personal notes (not tracked)

---

*All rules are automatically loaded by Claude Code. No @includes needed.*
