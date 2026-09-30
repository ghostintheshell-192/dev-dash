# DevDash Architecture

A public, conceptual overview. For the exact API surface see
[.development/api-design.md](../.development/api-design.md); for the live file
tree see [.development/ARCHITECTURE.md](../.development/ARCHITECTURE.md).

---

## What DevDash is

DevDash is a documentation-first project dashboard that collaborates with
Claude Code. It **manages documentation and context** — the project's own
docs plus the Claude Code configuration that applies to it — and deliberately
does **not** duplicate Claude Code's execution/automation capabilities.

> DevDash manages documentation and context. Claude Code manages execution
> and automation.

It is a Linux-first desktop application: **C++20 + Dear ImGui + SDL3 +
Vulkan**. (The original prototype was C#/Avalonia; the pivot is recorded in
[ADR-008](../.development/reference/decisions/008-pivot-to-cpp-imgui.md).)

---

## Layered architecture

The code under `app/src/` is split into layers with a strict dependency
direction (ADR-010). Each layer may only depend on the ones above it in this
table:

| Layer | Path | Responsibility | Depends on |
|-------|------|----------------|------------|
| **core** | `core/` | Pure value types (header-only), no behavior | nothing (just `<filesystem>`, `<string>`, …) |
| **services** | `services/` | Domain logic: file I/O, config resolution, diff/apply/promote, snapshots | core |
| **ui** | `ui/` | ImGui panels, shell, fonts, markdown renderer | core, services, ImGui |
| **platform** | `platform/` | SDL3/Vulkan/ImGui plumbing | SDL/Vulkan/ImGui — **not** ui/services |
| **app** | `app/` | Composition root + main loop | all layers |

The boundary is mechanically checkable: no `#include "ui/..."` in
`platform/`, no `#include "platform/..."` in `services/` or `core/`. The test
of the design: swapping the rendering backend should leave `core/` and
`services/` untouched. This separation is also what lets the services compile
into a standalone `dev-dash-services` library that the unit tests link
without the graphics stack.

Two conventions worth knowing:

- **"Panel as viewmodel"** instead of MVVM: each panel class in `ui/` holds
  its own state and receives services by constructor injection. No separate
  ViewModel layer.
- **Concrete services, no virtual interfaces**: tests use real temp dirs. A
  service is promoted to a virtual interface only when a real seam appears.

---

## Domain model (core/)

The pure value types that everything else operates on:

| Type | Represents |
|------|------------|
| `Project` | The directory under inspection + presence flags (`.claude/`, `.git`, `CLAUDE.md`). |
| `ConfigLayer` | A labeled directory holding Claude config: Global / Workspace / Project. |
| `EffectiveConfig` | The merged view, organized into **sections** (see below). |
| `Scaffold` | A user-managed config template under `~/.devdash/scaffolds/<name>/`. |
| `Snapshot` | A point-in-time backup under `~/.devdash/snapshots/<slug>/<timestamp>/`. |
| `DiffEntry` / `LineDiff` | File-level and line-level diff results between two trees. |

---

## Claude Code configuration model

Claude Code configuration is spread across three layers, merged with
**Project > Workspace > Global** precedence:

| Layer | Location |
|-------|----------|
| Global | `~/.claude/` |
| Workspace | `<workspace>/.claude/` (optional) |
| Project | `<project>/.claude/`, `CLAUDE.md` |

DevDash resolves these into an **`EffectiveConfig`**: instead of one flat
file list, the result is grouped into **section-aware** categories, each
carrying whether it is always in Claude's context or loaded on demand:

| Section (`ConfigSectionKind`) | What it gathers | Always in context? |
|------|-----------------|--------------------|
| `kClaudeMd` | `CLAUDE.md` + its `@`-includes (transitive) | yes |
| `kRules` | `.claude/rules/*.md` | yes |
| `kMemory` | memory files | yes |
| `kSkills` | available skills | on demand |
| `kAgents` | reusable agents | on demand |
| `kMcpServers` | MCP servers from `settings.json` | on demand |
| `kHooks` | hooks from `settings.json` | on triggering event |

Each section is produced by a dedicated **adapter** in `services/`
(`ClaudeMdAdapter`, `RulesAdapter`, `MemoryAdapter`, …) on top of two shared
helpers: `ConfigFileScanner` (directory scan + `@`-include resolution) and
`SettingsParser` (MCP servers + hooks out of `settings.json`).
`ConfigResolver` orchestrates the adapters into the final `EffectiveConfig`.

---

## Scaffolds, snapshots, and the apply/promote flow

DevDash treats config as something you can template, back up, and move
between a project and a scaffold:

- **Apply** (`ApplyEngine`): write selected files from a source tree (a
  scaffold or a snapshot) into a target project. Auto-snapshots the target
  first unless told otherwise.
- **Promote** (`PromoteEngine`): the reverse direction — copy selected files
  from a project *back* into a scaffold, so a refined project config becomes a
  reusable template.
- **Snapshot** (`SnapshotService`): save / list / restore / prune
  point-in-time backups. Restore is "apply with forced overwrite", and takes
  a `pre-restore` auto-snapshot first.
- **Diff** (`DiffEngine`): file-by-file (and line-level) comparison between
  two trees, feeding the Compare view.

The **source of truth** for the bundled scaffold is versioned under
`rsrc/project-scaffold/` (ADR-013); on the dev machine `~/.devdash/scaffolds/`
symlinks to it, so dogfooding writes straight into the git working tree.

---

## Rendering and the main loop

`platform/` owns the SDL3 window, the Vulkan context/swapchain/frame
resources, and the ImGui backend. `App` (the composition root) wires
everything in a fixed construction order and runs the frame loop; the
declaration order of its members drives LIFO destruction, which matters for
callbacks (e.g. the markdown renderer's link handler must outlive nothing
that captures it). The full construction/destruction graph is documented in
[api-design.md](../.development/api-design.md#cross-cutting).

The `ui/` shell provides the workspace: top bar, resizable navigation
sidebar (structure: config by layer, scaffolds, history), a central
dockspace (content: documents, diffs, tables), and a persistent status bar.
**Sidebar = structure, workspace = content.**

---

## Key decisions

The Architecture Decision Records live in
[.development/reference/decisions/](../.development/reference/decisions/):

- [ADR-001 — Technology stack](../.development/reference/decisions/001-stack-tecnologico.md)
- [ADR-002 — Symlink vs copy](../.development/reference/decisions/002-symlink-vs-copy.md)
- [ADR-003 — Local issue tracking](../.development/reference/decisions/003-issue-tracking-locale.md)
- [ADR-006 — Desktop vs VS Code extension](../.development/reference/decisions/006-desktop-vs-vscode-extension.md)
- [ADR-007 — Removal of the embedded terminal](../.development/reference/decisions/007-rimozione-terminale-embedded.md)
- [ADR-008 — Pivot to C++ / ImGui](../.development/reference/decisions/008-pivot-to-cpp-imgui.md)
- [ADR-009 — Markdown library (imgui_md)](../.development/reference/decisions/009-markdown-library-imgui-md.md)
- [ADR-010 — Layered architecture design](../.development/reference/decisions/010-architecture-design.md)
- [ADR-011 — Release and distribution](../.development/reference/decisions/011-release-and-distribution.md)
- [ADR-012 — Codebase-agnostic automation](../.development/reference/decisions/012-codebase-agnostic-automation.md)
- [ADR-013 — Scaffold source of truth](../.development/reference/decisions/013-scaffold-source-of-truth.md)
