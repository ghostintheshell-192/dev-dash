---
status: implemented
created: 2026-06-12
---

# feature-ui-overhaul

UI overhaul driven by the 2026-06-11 dogfooding findings: centralized theme
("Grafite & Ambra") plus legibility fixes in the scaffold/promote flows.

## Goals

### 1. Centralized theme (Grafite & Ambra)

- New `ui/theme.{h,cpp}`: a `Theme` struct holding the full palette
  (backgrounds, text, accent, semantic status colors) and style parameters
  (rounding, padding, borders). Applied to `ImGuiStyle` by the composition
  root after ImGui init (layer rules: `platform/` must not depend on `ui/`).
- Theme is **data, not constants**: a future theme switcher only needs more
  instances (out of scope now — single theme).
- Palette:
  - window `#181614`, panel `#1f1d1a`, active `#2a2722`, border `#353026`
  - text `#d8d3c7`, dim `#8a8478`
  - accent amber `#e0a64e` (focus, selection, headings, default marker)
  - semantic: added `#8fbf6e`, removed `#d9776a`, modified `#e0a64e`,
    missing/external `#b08ad6`
- Style character: "misto per contesto" — reading panes airy (generous
  padding/line spacing), tables/diffs dense (light zebra striping); rounding
  5px windows/widgets, 3px inside tables; borders only where they separate.
- All scattered hardcoded `ImVec4` in panels replaced by theme semantic
  colors — one meaning, one color, everywhere.

### 2. Dogfooding legibility fixes

- **Diff rows say where the file lives**: scaffold vs project location made
  explicit per row (finding: rsrc/runtime equivoco).
- **Promote confirmation shows the target scaffold**: the scaffold selector
  must read as a selector; confirmation dialog states the destination
  (finding: two promotes landed on PROVA1 unnoticed).
- **Entry points categorized**: `.development/automation/` files get their
  own section instead of falling into generic "Other".
- **Errors visible**: promote/apply failures surfaced prominently (status
  color + non-dismissable-by-accident placement), not buried.

### 3. Default scaffold marker (idea 2026-06-12-scaffold-default-marker)

- Verify apply engine does NOT copy `.devdash-default` into projects; add
  exclusion if needed.
- Add `.devdash-default` to `rsrc/project-scaffold/`.
- Case-insensitive scaffold sort; default scaffold visually marked (amber
  badge) in selector and lists.

## Non-goals

- Multiple themes / theme switcher UI (future).
- New panels or functional features beyond the fixes above.

## Addendum — scope grown during implementation (2026-06-12/13)

Iterating with live feedback, the overhaul went well beyond the original
three goals:

- **Workspace shell** (`ui/shell`): the fullscreen state machine replaced
  by top bar + resizable sidebar + central dockspace + persistent status
  bar (`StatusSink`). Documents and diffs open as dockable tabs.
- **VS Code-style sidebar** (`ui/sidebar`): collapsible sections with
  trees — Config (per-layer markers), Scaffolds (file trees, '...' and
  context menu: Compare / Set as default / New / Delete), History preview.
  Rule: sidebar = structure, workspace = content.
- **Compare view redesign**: renamed from "Scaffold diff"; one row per
  file, left checkbox, status pills, ghost-button actions, unchanged
  hidden by default, Apply/Promote always visible with (?) HelpMarkers.
- **Readable file diff**: full-row background tint + legend in words.
- **DPI scaling**: fonts and metrics follow SDL display content scale.
- **Repository additions**: `ListFiles`, `SetDefault`.
- Bugs fixed en route: diff viewer tab flicker (render decoupled from the
  opening panel); latent `build.sh` preset-resolution bug.

Deferred (idea notes): generic WinMerge-style compare
(`2026-06-12-generic-compare-operation.md`); dock layout persistence
(io.IniFilename) not yet enabled.

## Verification

- Build via `.development/automation/build.sh`; visual pass by Valentina on
  the real app (scaffold diff vs `dev-dash-standard` and `PROVA1`, a promote
  dry-run, markdown reading pane). Done iteratively throughout 2026-06-12/13.
