# DevDash

Personal dashboard for solo development with Claude Code.

## What is it

DevDash is a unified interface for managing software and creative projects, designed for solo developers working with AI assistance. It integrates:

- **Project navigation** across thematic workspaces
- **Documentation management** (.personal + docs)
- **Personal issue tracking** without the overhead of GitHub Issues
- **Claude Code configurations** displayed and editable
- **Claude Code sessions** launched with pre-loaded context

## Why it exists

When you work on multiple projects using spec-driven development and maintain structured documentation, you need to:

1. See everything in one place
2. Quickly navigate between projects and their documentation
3. Track personal issues that don't warrant GitHub
4. Manage Claude Code configurations spread across three levels
5. Launch Claude Code with the right context already loaded

DevDash solves these problems without reinventing the editor (you still use VS Code/Obsidian for editing).

## Stack

- **Framework**: Avalonia UI 11.3 (cross-platform)
- **Language**: C# / .NET 8
- **Pattern**: MVVM (CommunityToolkit.Mvvm)
- **Theme**: Fluent (dark mode)

## Status

🟡 **Prototype** - Functional UI with mock data, not yet connected to the real filesystem.

## Next steps

Development plan in `.personal/planning/roadmap.md` (6 phases from prototyping to polish).

## Structure

```text
dev-dash/
├── src/DevDash/             # Avalonia application
│   ├── Models/              # Data models
│   ├── ViewModels/          # MVVM ViewModels
│   ├── Views/               # XAML views
│   └── Services/            # Business logic
├── devdash-prototype.tsx    # UI prototype (reference)
├── docs/
│   ├── ARCHITECTURE.md      # Architecture overview
│   └── SETUP.md             # Installation guide
└── .personal/               # Private documentation (not committed)
    ├── planning/            # Roadmap, plans
    ├── reference/decisions/ # ADR
    └── active/              # Work in progress
```
