# DevDash - Project Instructions

## Project Overview

**Type**: C# / Avalonia Desktop Application
**Purpose**: Dashboard personale per gestire progetti e documentazione con Claude Code

## Tech Stack

- **Framework**: Avalonia UI 11.3
- **Language**: C# / .NET 8
- **Pattern**: MVVM (CommunityToolkit.Mvvm)
- **Theme**: Fluent (dark mode)

## Project Structure

```
dev-dash/
├── src/DevDash/           # Main application
│   ├── Models/            # Data models
│   ├── ViewModels/        # MVVM ViewModels
│   ├── Views/             # XAML views
│   └── Services/          # Business logic, filesystem access
├── docs/                  # Public documentation
└── .personal/             # Private planning & notes
```

## Build & Run

```bash
cd src/DevDash
dotnet build
dotnet run
```

## MVVM Conventions

- **ViewModels**: Inherit from `ViewModelBase`, use `[ObservableProperty]` for properties
- **Views**: AXAML files, bind to ViewModels via `DataContext`
- **Services**: Injected via constructor, interfaces in `Services/`
- **ViewLocator**: Auto-resolves Views from ViewModels by naming convention

## Naming Conventions

| Type | Convention | Example |
|------|------------|---------|
| ViewModel | `{Name}ViewModel` | `ProjectListViewModel` |
| View | `{Name}View.axaml` | `ProjectListView.axaml` |
| Model | `{Name}` | `Project`, `Workspace` |
| Service | `I{Name}Service` / `{Name}Service` | `IFileService` |

## Key Models (from prototype)

```csharp
// Workspace - contenitore di progetti
record Workspace(int Id, string Name, string Path, string Type, string Icon);

// Project - singolo progetto
record Project(string Id, string Name, string Path, string? Branch,
               bool HasPersonal, bool HasDocs, string? Language);

// PersonalFile - file in .personal/
record PersonalFile(string Name, string Path, FileType Type,
                    string? Priority, DateTime? Modified);
```

## UI Reference

Il prototipo React (`devdash-prototype.tsx`) definisce il layout target:

- **Sidebar**: Workspace switcher + lista progetti
- **Main area**: Tabs (.personal, docs, issues, ADR, config)
- **File tree**: Navigazione `.personal/` con expand/collapse
- **Terminal panel**: Area Claude Code (collapsible)
- **Settings modal**: Gestione workspace e config

## Current Phase

Fase 1 della roadmap: implementare filesystem reale.

Vedi `.personal/CURRENT-STATUS.md` per lo stato attuale.

---

*For coding standards, see `/data/repos/rules/coding-standards/csharp-dotnet.md`*
