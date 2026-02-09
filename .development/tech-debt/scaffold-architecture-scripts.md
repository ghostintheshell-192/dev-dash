---
priority: medium
status: open
---

# Include architecture scripts in project scaffold

## Problem

The `generate-architecture.sh` and `extract-summary.*` scripts are manually created per project.
The scaffold creates `.development/scripts/` but doesn't include these scripts.

## Proposed Solution

1. Add a generic `generate-architecture.sh` to the scaffold with template variables for the project-specific section
2. Include language-specific `extract-summary` scripts (`.sh` for C#, `.py` for Python)
3. Use `{PROJECT_NAME}`, `{FILE_GLOB}`, `{SOURCE_DIRS}` etc. as template placeholders
4. `ScaffoldService.ApplyTemplateReplacements()` already handles this pattern

## Affected Files

- `rsrc/workspace-scaffold/.development/scripts/generate-architecture.sh`
- `rsrc/workspace-scaffold/.development/scripts/extract-summary.sh` (C#)
- `rsrc/workspace-scaffold/.development/scripts/extract-summary.py` (Python)
- `src/DevDash/Services/ScaffoldService.cs` — add new template variables
