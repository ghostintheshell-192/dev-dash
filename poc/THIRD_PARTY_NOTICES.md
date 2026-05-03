# Third-Party Notices

The dev-dash PoC includes or depends on the following third-party components.
This file is updated as components are added.

## Germen Pulchrum

The build setup *and* the renderer code (`src/renderer.{h,cpp}`,
`src/deletion_queue.h`) of this PoC are **derived** from [Germen Pulchrum] by
Dario Passet (`DPD85/Germen`, MIT license).

- Source: <https://github.com/DPD85/Germen>
- Pinned baseline: commit `037827b` (2026-04-29)
- License: MIT (see upstream `LICENSE`)
- Adapted in `poc/src/`:
  - `Disegnatore.cpp` → `renderer.cpp` (translated to English; restructured into
    a `Renderer` class; stripped of custom font loading, theme system, ImPlot,
    DPI scaling, and i18n).
  - `CodaCancellazione.{h,cpp}` → `deletion_queue.h` (header-only; same RAII
    stack-of-deleters pattern, English identifiers).

The build setup itself is also adapted: Linux-only for now, Conan removed,
internationalization removed.

Strategy of consumption (PoC phase): copy with attribution. Adopted code is
re-styled to dev-dash conventions on the way in (see
`.claude/rules/coding-standards.md` § "C++ Conventions"). If Germen continues
to evolve actively post-PoC, we may switch to a `git subtree` model so we can
pull upstream improvements; this decision is deferred until the PoC is
validated.

[Germen Pulchrum]: https://github.com/DPD85/Germen

## Dear ImGui

- Source: <https://github.com/ocornut/imgui> (docking branch)
- Version: 1.92.6-docking
- License: MIT
- Pulled in at configure time via CPM (see `external/CMakeLists.txt`).

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
