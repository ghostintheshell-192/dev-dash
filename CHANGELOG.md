# Changelog

All notable changes to this project are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

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
  tarball on `v*` tags.
- Compiler warnings enabled (`-Wall -Wextra`) on first-party targets.

### Changed

- User documentation rewritten for the C++/ImGui/SDL3/Vulkan stack
  (`docs/SETUP.md`, `docs/ARCHITECTURE.md`).

[Unreleased]: https://github.com/ghostintheshell-192/dev-dash/commits/develop
