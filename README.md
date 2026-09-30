# DevDash

A desktop dashboard for projects you work on with
[Claude Code](https://code.claude.com). It shows the whole context Claude
will see for a project, and keeps the project's documentation and
configuration in order.

> DevDash manages documentation and context. Claude Code manages execution.

DevDash does not run Claude, wrap a terminal or duplicate what Claude Code
already does. It reads the files Claude Code reads and shows them together,
with where each one comes from.

**Status:** early, pre-release (0.1.0). Built first as a personal tool; Linux
only for now.

## What it does

- **Effective configuration.** For a project, lists everything Claude Code
  loads: the `CLAUDE.md` chain (user, ancestor directories, project, local)
  with its `@`-includes, rules, auto-memory, skills, subagents, MCP servers
  and hooks. Each entry shows its layer (Global, Ancestor, Project, Local);
  when the same name is defined at two layers, the losing one stays visible,
  marked as shadowed. Click an entry to read it.
- **Scaffolds.** Keep a reference project configuration, diff it against a
  project, apply it, or promote a project's improvements back into it.
- **Snapshots.** Every apply is preceded by a snapshot of the project's
  configuration, so it can be restored.
- **Markdown reader.** Documents open in dockable panels, following
  `@`-includes.

## Build

Needs GCC ≥ 12, CMake ≥ 3.28, Ninja, the Vulkan development headers, bison
and flex. Other dependencies (SDL3, Dear ImGui, MD4C, ImGuiDot, Graphviz…) are
fetched at configure time.

```bash
git clone https://github.com/ghostintheshell-192/dev-dash.git
cd dev-dash
bash .development/automation/bootstrap.sh   # once per clone: git hooks
.development/automation/build.sh
./app/build/linux-debug/src/dev-dash
```

Full prerequisites, install and packaging: [docs/SETUP.md](docs/SETUP.md).

## Stack

C++20, [Dear ImGui](https://github.com/ocornut/imgui) (docking) on
[SDL3](https://github.com/libsdl-org/SDL) + Vulkan,
[MD4C](https://github.com/mity/md4c) with
[imgui_md](https://github.com/DPD85/imgui_md) for markdown,
[ImGuiDot](https://github.com/DPD85/ImGuiDot) with Graphviz for diagrams. The reasons behind
the stack are in
[ADR-008](.development/reference/decisions/008-pivot-to-cpp-imgui.md).

## Documentation

- [docs/](docs/): setup and a conceptual overview of the architecture.
- [.development/](.development/): how the project is built: current status,
  specs, architecture decision records, known tech debt. DevDash is developed
  with the same documentation method it supports.

## Acknowledgements

Parts of the Vulkan plumbing are adapted from
[Germen Pulchrum](https://github.com/DPD85/Germen) by Dario. Third-party
licenses are listed in [app/THIRD_PARTY_NOTICES.md](app/THIRD_PARTY_NOTICES.md).
