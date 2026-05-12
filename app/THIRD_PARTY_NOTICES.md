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

## imgui_markdown

A single-header markdown renderer for Dear ImGui.

- Base library: <https://github.com/enkisoftware/imgui_markdown> by
  Juliette Foucaut & Doug Binks.
- License: zlib (see header inline).
- Vendored at: `external/imgui_markdown/imgui_markdown.h`.
- Pinned source: `mgerhardy/imgui_markdown` @ `214a836c` (the head of
  PR <https://github.com/enkisoftware/imgui_markdown/pull/43>,
  "extend markdown syntax support (tables and code)"). We follow the
  PR branch instead of upstream main because the PR adds fenced code
  blocks and tables — features needed to render typical CLAUDE.md /
  ADR / spec content. The PR has been open without maintainer review
  since 2026-01-22; if it lands upstream, switch the pin back to
  `enkisoftware/imgui_markdown` main.

The integration pattern (callback shape, `MarkdownConfig` setup, link
handler routed through `SDL_OpenURL`) was informed by Germen Pulchrum's
own markdown integration in PR <https://github.com/DPD85/Germen/pull/4>.

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
