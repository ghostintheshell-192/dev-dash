# Changelog

All notable changes to this project are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.1] - 2026-09-30

### Fixed

- The release tarball starts on Debian 12: it is built on Ubuntu 22.04 with
  GCC 12, and the release workflow fails if the binary needs a newer glibc
  or libstdc++ than Debian 12 provides. The 0.1.0 tarball required
  `GLIBC_2.38` and did not start there.

### Added

- Release dry run (`workflow_dispatch`): builds and checks the tarball
  without publishing it.

## [0.1.0] - 2026-09-30

First pre-release of the C++20 / Dear ImGui rewrite.

### Added

- Effective config view aligned with Claude Code's loading rules: root,
  local and ancestor `CLAUDE.md`, skills as `SKILL.md` directories, MCP
  servers from `~/.claude.json` and `.mcp.json`, shadowed entries kept
  visible.
- Diagram panel: DOT editor with a live diagram, rendered with ImGuiDot
  (Graphviz layout).

- Unit test suite (Catch2) covering `SnapshotService` and `ApplyEngine`;
  domain logic extracted into a `dev-dash-services` library.
- Versioned binary (`version.h`), two-step asset resolution, and bundled
  DejaVu Sans — the app runs from an installed prefix without relying on
  system font paths.
- `~/.devdash/{scaffolds,snapshots}` created at first run, with I/O errors
  surfaced on screen instead of failing silently.
- Install rules (`GNUInstallDirs`), a `dev-dash.desktop` entry, and CPack
  TGZ packaging (`dev-dash-X.Y.Z-linux-x86_64.tar.gz`).
- GitHub Actions: CI (build + test, warnings-as-errors) on push/PR to
  develop and main; release workflow producing a GitHub Release with the
  tarball on `v*` tags; `0.y.z` and `-`-suffixed tags are published as
  pre-releases.
- Compiler warnings enabled (`-Wall -Wextra`) on first-party targets.

### Changed

- User documentation rewritten for the C++/ImGui/SDL3/Vulkan stack
  (`docs/SETUP.md`, `docs/ARCHITECTURE.md`).

[Unreleased]: https://github.com/ghostintheshell-192/dev-dash/compare/v0.1.1...develop
[0.1.1]: https://github.com/ghostintheshell-192/dev-dash/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/ghostintheshell-192/dev-dash/releases/tag/v0.1.0
