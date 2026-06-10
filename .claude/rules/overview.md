# Project Overview

## DevDash

**DevDash** is a personal documentation-first project dashboard that collaborates with Claude Code. It manages documentation and context (project + Claude configuration) without duplicating Claude Code's execution capabilities.

- **Type**: Personal-first tool, built to be distributable (the multi-OS-portable stack is a deliberate choice; see ADR-011 for the growth path)
- **Platform**: Desktop (C++20 + Dear ImGui + SDL3 + Vulkan, Linux-first)
- **Status**: Layered skeleton implemented; wedge features next

## Philosophy

> DevDash manages documentation and context. Claude Code manages execution and automation.

DevDash is NOT a "Claude Code manager" — it's a dashboard that shows the complete context Claude will see, without duplicating functionality.

## Development Methodology

- **Functional minimalism**: Minimum complexity for current requirements
- **Incrementality**: One component at a time, test before proceeding
- **Responsiveness**: Non-blocking UI is a requirement
- **Effective simplicity**: Simplest solution that works

This project follows a **spec-driven development** approach:

1. **Specification First**: Write detailed specs in `.development/specs/`
2. **Implementation**: Code according to spec
3. **Documentation**: Keep `.development/CURRENT-STATUS.md` updated
4. **Session Handoffs**: Use `.memory-bank/` for continuity between sessions

## Architecture

**Layered split** under `app/src/` (ADR-010):

- `core/` — pure value types, no external dependencies
- `services/` — domain logic (file I/O, config resolution, diff/apply, snapshots)
- `ui/` — ImGui panel classes, font library, markdown renderer
- `platform/` — SDL3/Vulkan/ImGui plumbing
- `app/` — composition root + main loop

**"Panel as viewmodel"** instead of MVVM: each panel class in `ui/` holds its own
state and receives services via constructor injection. No separate ViewModel layer.

**Concrete services** (no virtual interfaces): test fixtures use real tmpdir.
Promote to virtual interface only when a concrete seam emerges.

## Technology Stack

- **C++20** — GCC on Linux, no extensions
- **Dear ImGui 1.92.6-docking** — immediate-mode UI
- **SDL3 3.2.20** — windowing, input, built from source via CPM
- **Vulkan** — rendering backend (via vk-bootstrap 1.3.302)
- **MD4C 0.5.3** — CommonMark parser
- **imgui_md** — vendored bridge MD4C → ImGui rendering
- **CMake ≥ 3.28** + Ninja

## Key Documents

- [.development/CURRENT-STATUS.md](.development/CURRENT-STATUS.md) - Current project state
- [.development/api-design.md](.development/api-design.md) - Definitive API design (companion to ADR-010)
- [.development/INDEX.md](.development/INDEX.md) - Auto-generated navigation
- [.development/specs/](.development/specs/) - Feature specifications
- [.development/tech-debt/](.development/tech-debt/) - Known issues
- [.development/reference/decisions/](.development/reference/decisions/) - Architecture Decision Records
