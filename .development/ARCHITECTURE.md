# Architecture Reference

Quick reference for navigating the DevDash codebase.
For detailed documentation, see `docs/`.

> **Status note**: as of 2026-05-10, DevDash has pivoted from .NET 8 + Avalonia
> to C++20 + Dear ImGui (see [ADR-008](reference/decisions/008-pivot-to-cpp-imgui.md)).
> The current code lives under `poc/` (the validated proof-of-concept). The
> legacy .NET tree under `src/DevDash/` is preserved at tag `legacy/avalonia-final`
> and will be removed from `develop` in a dedicated branch.
> The layered split (platform / services / ui / core / app) for the real project
> is documented in [ADR-010](reference/decisions/010-architecture-design.md) and
> [api-design.md](api-design.md). Implementation (refactor of `poc/` → `app/`) is
> a separate session. Until then this document describes the **PoC layout** as it
> exists today.

## Layer Overview (current PoC under `poc/`)

| Layer | Path | Purpose |
|-------|------|---------|
| Entry point | `poc/src/main.cpp` | `int main` → `Renderer::Run()` |
| Application (god class) | `poc/src/renderer.{h,cpp}` | SDL3 init, Vulkan setup chain (instance/device/swapchain/render-pass/framebuffers/cmd-pool/sync), ImGui init, main loop, markdown panel state (`_panels`/`_pendingPanels`), `OpenPanel`, `RenderMarkdownWindow`, `PreprocessImports`. Slated for split in ADR-010. |
| Markdown rendering | `poc/src/rendering/markdown_r.{h,cpp}` | `Rendering::MarkdownRenderer` deriving from `imgui_md`; overrides `get_font`, `open_url`, `SPAN_CODE`, `BLOCK_CODE`. |
| RAII utility | `poc/src/deletion_queue.h` | LIFO stack of cleanup callbacks (adapted from Germen Pulchrum). |
| External deps (managed) | `poc/external/CMakeLists.txt` | CPM packages: SDL3, vk-bootstrap, Dear ImGui (download-only, built locally), MD4C. Static targets: `ImGui`, `ImGui-SDL3`, `ImGui-Vulkan`, `ImGui-MD`. |
| External deps (vendored) | `poc/external/imgui_md/` | Bridge MD4C → ImGui (Dmitry Mekhontsev, MIT) with local patches for ImGui 1.92.x. |
| ImGui config | `poc/external/imgui-config.h` | Routed via `IMGUI_USER_CONFIG`. |
| Build dispatch | `poc/cmake/` | `get_cpm.cmake` (CPM bootstrap), `compilers/gcc.cmake` (Linux/GCC options). |
| Assets | `poc/assets/fonts/` | IBM Plex Sans family + DejaVu Sans fallback (loaded via `SDL_GetBasePath()`). |

**Pattern note**: the PoC is intentionally monolithic (single `Renderer` class) to validate the stack. The real project will split responsibilities — see ADR-010.

## Key Decisions

- [ADR-001: Stack Tecnologico (C# + Avalonia)](reference/decisions/001-stack-tecnologico.md) — **Superseded by ADR-008**
- [ADR-002: Symlink vs Copy](reference/decisions/002-symlink-vs-copy.md)
- [ADR-003: Issue Tracking Locale](reference/decisions/003-issue-tracking-locale.md)
- [ADR-006: Desktop vs VS Code Extension](reference/decisions/006-desktop-vs-vscode-extension.md)
- [ADR-007: Rimozione Terminale Embedded](reference/decisions/007-rimozione-terminale-embedded.md)
- [ADR-008: Pivot dello stack — C++/Dear ImGui](reference/decisions/008-pivot-to-cpp-imgui.md)
- [ADR-009: Libreria markdown — imgui_md + MD4C](reference/decisions/009-markdown-library-imgui-md.md)
- [ADR-010: Architettura del progetto vero — split layered](reference/decisions/010-architecture-design.md)

## Project Tree (current PoC)

```
poc/
├── CMakeLists.txt                      # Project setup, compiler dispatch, CPM bootstrap, subdirs
├── CMakePresets.json                   # linux-debug / linux-release configs
├── THIRD_PARTY_NOTICES.md              # Attribution for borrowed code (Germen) + vendored deps (imgui_md)
├── .gitignore                          # build/ excluded
│
├── cmake/
│   ├── get_cpm.cmake                   # CPM (CMake Package Manager) bootstrap
│   └── compilers/
│       └── gcc.cmake                   # Warnings, sanitizer hooks, -fdiagnostics-color
│
├── external/
│   ├── CMakeLists.txt                  # CPM deps + ImGui static lib targets
│   ├── imgui-config.h                  # IMGUI_USER_CONFIG (locally vendored ImGui customizations)
│   └── imgui_md/
│       ├── imgui_md.h                  # Bridge header (vendored, MIT)
│       └── imgui_md.cpp                # Bridge impl (vendored, MIT, with ImGui 1.92.x patches inline)
│
├── src/
│   ├── CMakeLists.txt                  # `dev-dash-poc` executable target + asset copy POST_BUILD
│   ├── main.cpp                        # int main → Renderer().Run()
│   ├── renderer.h                      # God class declaration
│   ├── renderer.cpp                    # SDL3+Vulkan+ImGui+loop+panels+import preprocessing
│   ├── deletion_queue.h                # RAII LIFO cleanup stack
│   └── rendering/
│       ├── markdown_r.h                # Rendering::MarkdownRenderer : imgui_md
│       └── markdown_r.cpp              # Font system, open_url (handles claudeimport://)
│
├── assets/
│   └── fonts/
│       ├── IBMPlexSans-Regular.ttf     # SIL OFL 1.1
│       ├── IBMPlexSans-Italic.ttf
│       ├── IBMPlexSans-Bold.ttf
│       └── (loaded via SDL_GetBasePath() at runtime)
│
└── rsrc/
    └── test.md                         # Sample document for markdown panel testing
```

## Legacy tree (preserved at `legacy/avalonia-final`)

The `.NET 8 + Avalonia` codebase under `src/DevDash/` is **frozen**: it no
longer builds in `develop` (CI disabled, NuGet packages still resolvable but
unused). To inspect: `git checkout legacy/avalonia-final`. To remove from
`develop`: dedicated cleanup branch (tracked in `CURRENT-STATUS.md`).

---

*Manually maintained until `.development/scripts/generate-architecture.sh` is updated for C++ (it currently scans `*.cs` files only — see cleanup tasks in `CURRENT-STATUS.md`).*
