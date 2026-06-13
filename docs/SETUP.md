# Setup DevDash

How to build, install, and first-run DevDash on Linux.

> DevDash is a C++20 desktop app (Dear ImGui + SDL3 + Vulkan), Linux-first.
> The stack changed from the original .NET/Avalonia prototype — see
> [ADR-008](../.development/reference/decisions/008-pivot-to-cpp-imgui.md).

---

## Prerequisites

| Component | Requirement | Notes |
|-----------|-------------|-------|
| Compiler | **GCC ≥ 12** | Only GCC is supported on Linux for now. |
| Build system | **CMake ≥ 3.28** + **Ninja** | Presets need CMake 3.28. |
| Graphics | **Vulkan**: loader + headers + a working driver (ICD) | `find_package(Vulkan)` needs the dev headers; the app needs a GPU driver at runtime. |
| Misc | **Git**, network access | Dependencies are fetched from GitHub at configure time (CPM). |

SDL3 is **built from source** via CPM (Debian 12 doesn't ship `libsdl3-dev`
yet), so the usual system headers SDL needs to build must be present —
typically X11/Wayland and input/keyboard dev packages.

On Debian/Ubuntu, an indicative set (exact names vary by release):

```bash
sudo apt install \
    build-essential cmake ninja-build git pkg-config \
    libvulkan-dev vulkan-validationlayers mesa-vulkan-drivers \
    libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev \
    libxfixes-dev libwayland-dev wayland-protocols libxkbcommon-dev
```

Adjust per your distro and GPU (e.g. proprietary NVIDIA drivers provide their
own Vulkan ICD).

---

## Clone and bootstrap

```bash
git clone https://github.com/ghostintheshell-192/dev-dash.git
cd dev-dash

# One-time per clone: activates the project git hooks (branch protection,
# spec workflow, derived-docs regeneration) and makes the automation entry
# points executable.
bash .development/automation/bootstrap.sh
```

---

## Build and run

The repo exposes stack-agnostic entry points under `.development/automation/`
(ADR-012); they wrap the CMake/CTest/CPack details.

```bash
# Build (defaults to the linux-debug preset)
.development/automation/build.sh

# Run from the build tree
./app/build/linux-debug/src/dev-dash

# Run the unit tests (Catch2 via ctest)
.development/automation/test.sh
```

Equivalent raw CMake, if you prefer (presets live in `app/`, so `--build`
must run from there):

```bash
cmake --preset linux-debug -S app
(cd app && cmake --build --preset linux-debug)
```

Available presets: `linux-debug`, `linux-release`. Unit tests build by
default; pass `-DDEVDASH_BUILD_TESTS=OFF` to skip them.

---

## Install

Install into any prefix with the standard CMake install step. The layout
follows `GNUInstallDirs`:

```bash
.development/automation/build.sh linux-release
cmake --install app/build/linux-release --prefix /usr/local
```

Result:

```
<prefix>/bin/dev-dash
<prefix>/share/dev-dash/assets/...        # bundled fonts, etc.
<prefix>/share/applications/dev-dash.desktop
```

The executable resolves its assets relative to its own location: first
`<exe>/assets`, then `<exe>/../share/dev-dash/assets` — so both the build
tree and an installed prefix work without hardcoded paths.

### Tarball

```bash
.development/automation/build.sh linux-release
(cd app/build/linux-release && cpack)
# → dev-dash-0.1.0-linux-x86_64.tar.gz
```

---

## First run

On startup DevDash prepares its working directory:

```
~/.devdash/
├── scaffolds/    # user-managed scaffolds (config templates)
└── snapshots/    # point-in-time backups of project config
```

These are created automatically if absent. Any I/O problem (e.g. a
non-writable `$HOME`) is surfaced in a dismissable on-screen warning rather
than failing silently.

---

## Troubleshooting

**`Only GCC is supported on Linux`** — configure with GCC, e.g.
`CC=gcc CXX=g++ .development/automation/build.sh`.

**Vulkan errors at startup / black window** — verify a working driver:
`vulkaninfo | head` should list a device. Install the right
`mesa-vulkan-drivers` (or vendor driver) for your GPU.

**SDL3 fails to configure/build** — a system dev header SDL needs is missing;
install the X11/Wayland packages listed above.

**Fonts look wrong / missing glyphs** — the bundled fonts weren't found.
Check that `assets/fonts/` sits next to the executable (build tree) or under
`share/dev-dash/assets/fonts/` (installed).

---

## See also

- [ARCHITECTURE.md](ARCHITECTURE.md) — how the app is structured
- [.development/api-design.md](../.development/api-design.md) — definitive API surface
- [.development/reference/decisions/](../.development/reference/decisions/) — ADRs
