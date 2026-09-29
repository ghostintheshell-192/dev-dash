# Third-Party Notices

The dev-dash app includes or depends on the following third-party components.
This file is updated as components are added.

## Germen Pulchrum

The original build setup and parts of the platform layer of dev-dash are
**derived** from [Germen Pulchrum] by Dario Passet (`DPD85/Germen`, MIT
license). The monolithic `Renderer` class adapted from Germen during the
PoC phase has since been refactored into the layered architecture under
`app/src/` (see ADR-010); the residual derived code is the deletion queue.

- Source: <https://github.com/DPD85/Germen>
- Pinned baseline: commit `037827b` (2026-04-29)
- License: MIT (see upstream `LICENSE`)
- Adapted in `app/src/platform/`:
  - `CodaCancellazione.{h,cpp}` → `deletion_queue.h` (header-only; same RAII
    stack-of-deleters pattern, English identifiers).

The build setup itself is also adapted: Linux-only for now, Conan removed,
internationalization removed.

Strategy of consumption: copy with attribution. Adopted code is re-styled
to dev-dash conventions on the way in (see
`.claude/rules/coding-standards.md` § "C++ Conventions").

[Germen Pulchrum]: https://github.com/DPD85/Germen

## Dear ImGui

- Source: <https://github.com/ocornut/imgui> (docking branch)
- Version: 1.92.6-docking
- License: MIT
- Pulled in at configure time via CPM (see `external/CMakeLists.txt`).

## MD4C

CommonMark / Markdown parser written in C. Drives the parse-event
stream that `imgui_md` consumes to render Markdown via Dear ImGui.

- Source: <https://github.com/mity/md4c>
- Version: 0.5.3 (tag `release-0.5.3`)
- License: MIT
- Pulled in at configure time via CPM as a static library. The
  companion targets `md2html` (executable) and `md4c-html` (HTML
  renderer) are disabled — we only need the parser core.

## imgui_md

Bridge library that renders Markdown via Dear ImGui by consuming MD4C
parser events. Replaces the previous `imgui_markdown` integration (the
single-header `enkisoftware/imgui_markdown` library plus `mgerhardy`'s
fork for tables and fenced code blocks), dropped because MD4C is a
better-maintained CommonMark parser and gives proper table support
out of the box.

- Base library: <https://github.com/mekhontsev/imgui_md> by
  Dmitry Mekhontsev (upstream, inactive since 2022).
- Active fork: <https://github.com/DPD85/imgui_md> by Dario Passet —
  tracks Dear ImGui API changes (e.g. the `PushFont(size)` signature
  in 1.92.x) and ships a standalone `CMakeLists.txt` that defines the
  `imgui_md` target and gates ImGui/MD4C build blocks behind
  `if(NOT TARGET ...)` so consumers that already provide those
  targets aren't duplicated.
- License: MIT (same as upstream).
- Pulled in at configure time via CPM (see `external/CMakeLists.txt`).
  During co-development of the fork the CPM block points at a local
  clone (`SOURCE_DIR /data/repos/imgui_md`); the stable form using
  `GITHUB_REPOSITORY DPD85/imgui_md` + `GIT_TAG <SHA>` is committed
  out alongside as a reference and is restored once fork PRs land.

The integration pattern (callback shape, link handler routed through
`SDL_OpenURL`) was originally informed by Germen Pulchrum's own
markdown integration in PR <https://github.com/DPD85/Germen/pull/4>,
and the `MarkdownRenderer` class in `app/src/ui/` derives `imgui_md`
to customize fonts and link handling.

## ImGuiDot

Draws Graphviz diagrams, written in the DOT language, with the Dear ImGui
draw list. Base of the code graph (`feature-code-graph`).

- Source: <https://github.com/DPD85/ImGuiDot> by Dario Passet
- License: MIT
- Pulled in at configure time via CPM, currently pinned to a commit of the
  fork <https://github.com/ghostintheshell-192/ImGuiDot> (branch
  `feature/style-colours`) pending its merge upstream. Built with
  `BUILD_IMGUI OFF`: it links dev-dash's own ImGui target.

## Graphviz

Parses DOT and computes the diagram layout for ImGuiDot.

- Source: <https://gitlab.com/graphviz/graphviz>
- Version: 15.1.0
- License: Eclipse Public License 2.0 (EPL-2.0)
- Pulled in by ImGuiDot via CPM and built from source (needs `bison` and
  `flex`), with a small patch to its Flex/Bison CMake setup
  (`Graphviz-FlexBison.patch` in ImGuiDot). Linked statically into the
  `dev-dash` binary. Under EPL-2.0 a binary distribution must say where the
  Graphviz source is available: the URL and version above, unmodified apart
  from that build patch.

## vk-bootstrap

- Source: <https://github.com/charles-lunarg/vk-bootstrap>
- Version: 1.3.302 (last release of the 1.3.x branch; pinned to match the
  Vulkan 1.3 headers shipped on Debian 12 / bookworm-backports).
- License: MIT
- Pulled in at configure time via CPM.

## CPM.cmake

- Source: <https://github.com/cpm-cmake/CPM.cmake>
- Version: 0.40.8
- License: MIT
- Downloaded on first configure into the build directory (see `cmake/get_cpm.cmake`).

## SDL3

- Source: <https://github.com/libsdl-org/SDL>
- Version: 3.2.20 (tag `release-3.2.20`)
- License: Zlib
- Built from source via CPM as a static library. Debian 12 (bookworm) does
  not yet ship `libsdl3-dev`; the in-tree build avoids the dependency on a
  specific OS version.

## Vulkan

Provided by the host system (`libvulkan-dev` on Debian/Ubuntu). Not
redistributed in this repo. The Vulkan loader is dynamically linked at
runtime as usual.

## nlohmann/json

Header-only JSON library. Used by `SettingsParser` to read Claude
Code's `settings.json` (MCP servers and hooks configuration).

- Source: <https://github.com/nlohmann/json>
- Version: 3.11.3
- License: MIT
- Pulled in at configure time via CPM. `JSON_BuildTests` and
  `JSON_Install` disabled.
