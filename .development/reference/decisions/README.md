# Architecture Decision Records

This folder contains key architectural decisions made for the project.

## Format

Each decision document should include:

1. **Date**: When the decision was made
2. **Status**: Active, Superseded, Deprecated
3. **Context**: Why the decision was needed
4. **Decision**: What was decided
5. **Rationale**: Why this option was chosen
6. **Consequences**: Trade-offs and implications

## Naming Convention

`NNN-short-description.md`

Example: `001-error-handling-philosophy.md`

## Index

| ADR | Title | Status |
|-----|-------|--------|
| [001](001-stack-tecnologico.md) | Stack tecnologico - C# + Avalonia | **Superseded by [008](008-pivot-to-cpp-imgui.md)** |
| [002](002-symlink-vs-copy.md) | Symlink vs Copy | Active |
| [003](003-issue-tracking-locale.md) | Issue tracking locale | Active |
| 004 | Claude Integration (read-only per config native) | *Pending* |
| 005 | Path configurabili | *Pending* |
| [006](006-desktop-vs-vscode-extension.md) | DevDash Desktop vs VS Code Extension | Active |
| [007](007-rimozione-terminale-embedded.md) | Rimozione del Terminale Embedded | Active |
| [008](008-pivot-to-cpp-imgui.md) | Pivot dello stack — da C#/Avalonia a C++/Dear ImGui | Active |
| [009](009-markdown-library-imgui-md.md) | Libreria markdown — imgui_md + MD4C | Active |
| [010](010-architecture-design.md) | Architettura del progetto vero — split layered | Active |
