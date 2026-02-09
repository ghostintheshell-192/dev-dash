# DevDash - Project Instructions

## Project Overview

**Type**: C# / Avalonia Desktop Application
**Purpose**: Documentation-first project dashboard that collaborates with Claude Code

## Philosophy

> DevDash manages documentation and context. Claude Code manages execution and automation.

DevDash is NOT a "Claude Code manager" — it's a dashboard that shows the complete context Claude will see, without duplicating functionality.

## Tech Stack

- **Framework**: Avalonia UI 11.3
- **Language**: C# / .NET 8
- **Pattern**: MVVM (CommunityToolkit.Mvvm)
- **Theme**: Fluent (dark mode)

## Project Structure

```text
dev-dash/
├── src/DevDash/           # Main application
│   ├── Models/            # Data models
│   ├── ViewModels/        # MVVM ViewModels
│   ├── Views/             # XAML views
│   └── Services/          # Business logic, filesystem access
├── docs/                  # Public documentation
│   ├── ARCHITECTURE.md    # System architecture
│   └── SETUP.md           # Installation guide
└── .personal/             # Private planning & notes
    └── planning/          # Feature specs
```

## Build & Run

```bash
cd src/DevDash
dotnet build
dotnet run
```

## MVVM Conventions

- **ViewModels**: Inherit from `ViewModelBase`, use `[ObservableProperty]`
- **Views**: AXAML files, bind to ViewModels via `DataContext`
- **Services**: Interfaces in `Services/`, injected via constructor
- **ViewLocator**: Auto-resolves Views from ViewModels by naming convention

## Naming Conventions

| Type      | Convention                         | Example                |
|-----------|------------------------------------| -----------------------|
| ViewModel | `{Name}ViewModel`                  | `ProjectListViewModel` |
| View      | `{Name}View.axaml`                 | `ProjectListView.axaml`|
| Model     | `{Name}`                           | `Project`, `Workspace` |
| Service   | `I{Name}Service` / `{Name}Service` | `IFileService`         |

## Claude Integration Principles

DevDash interacts with Claude Code configuration at three levels:

| Level     | What                                 | DevDash Access |
|-----------|--------------------------------------|----------------|
| Global    | `~/.claude/` (agents, commands)      | Read-only      |
| Workspace | `rules/` (standards, workflows)      | Read/Write     |
| Project   | `.claude/settings.json`, `.mcp.json` | Read-only      |
| Project   | `CLAUDE.md`, `.personal/`, `docs/`   | Read/Write     |

**Key services for Claude integration:**

- `IPathResolver` — Configurable path resolution
- `IClaudeConfigService` — Read Claude configs (global, project)
- `IWorkspaceConfigService` — Read/write workspace rules

## Current Focus

Implementing Claude Context Panel — unified view of what Claude "sees" when opening a project.

See `.personal/planning/feature-claude-context.md` for the spec.

## Key Documents

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — System architecture
- [.personal/CURRENT-STATUS.md](.personal/CURRENT-STATUS.md) — Current state
- [.personal/planning/feature-claude-context.md](.personal/planning/feature-claude-context.md) — Claude integration spec
