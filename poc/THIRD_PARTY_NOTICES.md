# Third-Party Notices

The dev-dash PoC includes or depends on the following third-party components.
This file is updated as components are added.

## Build & dependency layout

The build setup of this PoC is **derived** from [Germen Pulchrum] by Dario Pacchi
(`DPD85/Germen`, MIT license). Files originally in Italian were translated into
English; the structure was simplified (Linux-only for now, Conan removed,
internationalization removed). The original template can be found at
<https://github.com/DPD85/Germen>.

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
