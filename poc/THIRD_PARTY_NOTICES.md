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
- Version: 1.4.336
- License: MIT
- Pulled in at configure time via CPM.

## CPM.cmake

- Source: <https://github.com/cpm-cmake/CPM.cmake>
- Version: 0.40.8
- License: MIT
- Downloaded on first configure into the build directory (see `cmake/get_cpm.cmake`).

## SDL3 / Vulkan

Provided by the host system (apt packages on Debian/Ubuntu:
`libsdl3-dev`, `libvulkan-dev`, `vulkan-utility-libraries-dev`).
Not redistributed in this repo.
