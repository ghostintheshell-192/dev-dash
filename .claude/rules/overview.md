# Project Overview

## DevDash

**DevDash** is a personal documentation-first project dashboard that collaborates with Claude Code. It manages documentation and context (project + Claude configuration) without duplicating Claude Code's execution capabilities.

- **Type**: Personal tool (closed source, single-user)
- **Platform**: Cross-platform desktop (.NET 8 + Avalonia UI)
- **Status**: Early development (v0.2.x)

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

- **MVVM Pattern**: UI/ViewModel separation via CommunityToolkit.Mvvm
- **Service Layer**: Interfaces in `Services/`, injected via constructor
- **ViewLocator**: Auto-resolves Views from ViewModels by naming convention

## Technology Stack

- **.NET 8** + **C# 12**
- **Avalonia UI 11.x** - Cross-platform native UI (Fluent theme, dark mode)
- **CommunityToolkit.Mvvm** - MVVM source generators
- **LiveMarkdown.Avalonia** - Markdown rendering

## Key Documents

- [.development/CURRENT-STATUS.md](.development/CURRENT-STATUS.md) - Current project state
- [.development/INDEX.md](.development/INDEX.md) - Auto-generated navigation
- [.development/specs/](.development/specs/) - Feature specifications
- [.development/tech-debt/](.development/tech-debt/) - Known issues
- [.development/reference/decisions/](.development/reference/decisions/) - Architecture Decision Records
