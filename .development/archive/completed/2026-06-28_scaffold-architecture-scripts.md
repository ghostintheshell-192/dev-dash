---
priority: medium
status: resolved
resolved: 2026-06-28
---

# Include architecture scripts in project scaffold

## Problem

`generate-architecture.sh` and `extract-summary.sh` are dev-dash-specific and
are NOT shipped in the scaffold (`rsrc/project-scaffold/.development/scripts/`
ships only the generic generators). `generate-architecture.sh` in particular
carries a "Project Configuration" section hardcoded to dev-dash
(`PROJECT_NAME="DevDash"`, `SOURCE_DIRS=app/src`, a `generate_project_header()`
emitting dev-dash's Layer Overview). A project created from the scaffold gets
no architecture-map generation and no ADR-list discoverability.

## Proposed Solution

1. Add a generic `generate-architecture.sh` to the scaffold with the
   "Project Configuration" section as template placeholders
   (`{PROJECT_NAME}`, `{FILE_GLOBS}`, `{SOURCE_DIRS}`, project-header block).
2. Ship language-specific `extract-summary` variants (`.sh` already covers
   C#/C++; add `.py` for Python, etc.).
3. Have the scaffold-application path substitute the placeholders.
   NOTE: the original design pointed at `ScaffoldService.ApplyTemplateReplacements()`
   in the .NET codebase, removed at the C++ pivot (ADR-008). The equivalent
   today lives in `app/src/services/` (`ScaffoldRepository` / `PromoteEngine`),
   and the template-replacement step must be verified or implemented there.

## Affected Files

- `rsrc/project-scaffold/.development/scripts/generate-architecture.sh` (new, templated)
- `rsrc/project-scaffold/.development/scripts/extract-summary.sh`
- `app/src/services/scaffold_repository.{h,cpp}` / `promote_engine.{h,cpp}` — template-variable handling

## Related

Part of the broader scaffold ↔ ADR-model alignment deferred on 2026-06-14
(see `../../.memory-bank/ideas/2026-06-14-scaffold-adr-model-alignment.md`),
which also covers shipping the ADR model itself (`_TEMPLATE.md`, README,
`key-decisions.md` + `@include` wiring) and the PromoteEngine coverage gap.

## Resolution (2026-06-28)

- **Architecture scripts shipped** (branch `chore/scaffold-adr-alignment`,
  merged to `develop`): the scaffold now carries a templated
  `generate-architecture.sh` (with `{PROJECT_NAME}`/`{FILE_GLOBS}`/`{SOURCE_DIRS}`/
  `{PROJECT_LAYER_OVERVIEW}` placeholders) and `extract-summary.sh`. Fixed a
  latent bug: `docs-update.sh` referenced a generator the scaffold did not ship.
- **Template-replacement** (the `ScaffoldService.ApplyTemplateReplacements()`
  successor): verified absent by design, then re-decided as explicit
  manifest-guided substitution → **ADR-015** + `feature-scaffold-templating`
  (planned). No longer a tech-debt item: it is now planned feature work.
