# Legacy Workspace Rules - Backup

This directory contains backups of the old workspace-level `.rules/` system that was used before adopting Claude Code's official `.claude/rules/` pattern.

## History

Originally, DevDash used a workspace-level `.rules/` directory to share coding standards, workflows, and templates across multiple projects in the same workspace. This was an early attempt to avoid duplication and maintain consistency.

However, Claude Code officially supports:
- `.claude/rules/` at project level (auto-loaded, with path-specific support)
- `~/.claude/rules/` at user level (global personal preferences)

The workspace-level approach has been deprecated in favor of the official pattern.

## Contents

### `workspace-root-rules/`
Backup of `/data/repos/.rules/` - the workspace root configuration

### `scaffold-rules/`
Backup of `/data/repos/dev-dash/rsrc/workspace-scaffold/.rules/` - the scaffold template

## Migration Path

Content from these directories can be used to:
1. Populate project-level `.claude/rules/` when initializing new projects
2. Extract reusable templates for DevDash's scaffold system
3. Inform coding standards selection based on project type

## Note

These files are kept as reference material and templates. They are not actively used by the current system.

---

*Backed up: 2026-02-09*
