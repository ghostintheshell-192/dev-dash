# DevDash - Current Status

*Last updated: 2026-01-01*

## Project Phase

**Phase**: UI funzionante con workspace portabile, detection nuova struttura

Current version: **v0.2.0** (workspace portabile)

## Recent Work

### Session 2026-01-01: Nuova Struttura Workspace Portabile

**Struttura Workspace Aggiornata:**
- `.rules/` invece di `rules/` (hidden folder, portabile)
- `.memory-bank/` per memoria operativa (handoff tra sessioni)
- `CLAUDE.md` rimane nella root come entry point
- Bootstrap file in `.rules/bootstrap-coding.md` o `bootstrap-writing.md`

**Modelli Aggiornati:**
- `Workspace.cs`: aggiunte proprieta `HasRules`, `HasMemoryBank`, `HasClaudeMd`, `BootstrapType`
- `AppSettings.cs`: aggiunta proprieta `WorkspaceType` ("coding" | "writing")

**Servizi Aggiornati:**
- `WorkspaceService.cs`: nuovo metodo `DetectBootstrapType()`, detection struttura `.rules/`
- `FileSystemService.cs`: nuovi metodi `GetMemoryBankTree()`, `GetRulesTree()`
- `IFileSystemService.cs`: interfaccia aggiornata con nuovi metodi

**ViewModel Aggiornato:**
- `MainWindowViewModel.cs`: nuove proprieta `WorkspaceType`, `IsCodingWorkspace`, `WorkspaceHasRules`, `WorkspaceHasMemoryBank`, `WorkspaceHasClaudeMd`

**UI Aggiornata:**
- Settings panel: radio button Coding/Writing per workspace type
- Settings panel: indicatori stato workspace con pallini colorati (verde=presente, grigio=assente)
- Project list: badge linguaggio visibile accanto al nome progetto

**Nuovo Converter:**
- `BoolToColorConverter.cs`: converte bool in colore (#22c55e verde / #64748b grigio)

---

### Session 2025-12-30: Rimozione Terminale Embedded

**Decisione Architetturale:**
- Rimosso terminale embedded - Cancellata feature dopo revisione architetturale
  - Eliminati file: TerminalService, TerminalViewModel, TerminalView, AnsiParser (7 file)
  - Ripristinati file modificati: App.axaml.cs, DevDash.csproj, MainWindowViewModel, MainWindow.axaml
  - Rimossa dipendenza Pty.Net
  - Compilazione: OK (0 errori, 0 warning)

**Documentazione:**
- Creato [ADR-007: Rimozione del Terminale Embedded](reference/decisions/007-rimozione-terminale-embedded.md)
- Marcato `feature-embedded-terminal.md` come **cancelled**
- Aggiornato README degli ADR

**Rationale:**
- Terminale embedded era sovra-ingegnerizzazione (gestione PTY, ANSI parsing, thread safety)
- DevDash deve focalizzarsi su **documentazione e contesto**, non esecuzione
- Terminale di sistema (Windows Terminal, ecc.) e piu che sufficiente
- Allineamento con piano futuro di VS Code Extension (che ha gia terminale integrato)

---

### Session 2025-12-11: MultiSidebar Implementata + Feature Planning

**Implementato:**
- **MultiSidebar Control** - Sistema scalabile per sidebar collapsibili
  - Icon bar verticale (stile VS Code)
  - Radio-button behavior (una sola sidebar aperta)
  - Animazioni smooth di apertura/chiusura
  - DataTemplate support per contenuto flessibile
  - Fixed DataContext propagation issue (con aiuto Opus)

- **SidebarItem Model** - ContentTemplate (IDataTemplate) per rendering dinamico

- **Applicato a:**
  - Projects sidebar (workspace switcher + project list)
  - Files sidebar (file tree view)

**Feature Planning:**
- `feature-markdown-rendering.md` - Rendering/editing markdown (MVP: 1 giorno)
- `feature-issue-management.md` - Kanban board per tech-debt issues
- `feature-embedded-terminal.md` - **CANCELLED** (vedi ADR-007)

---

### Session 2025-12-04: Struttura base creata

**Models implementati:**
- `Workspace.cs` - contenitore di progetti
- `Project.cs` - singolo progetto con metadata
- `PersonalFile.cs` - nodo file tree .personal
- `FileType.cs` - enum File/Folder + Priority
- `ConfigFile.cs` - configurazioni Claude Code

**Services implementati:**
- `IFileSystemService` / `FileSystemService` - lettura filesystem, tree .personal
- `IWorkspaceService` / `WorkspaceService` - workspace/progetti, detect linguaggio
- `IConfigurationService` / `ConfigurationService` - config Claude Code

**ViewModels:**
- `MainWindowViewModel` - stato globale app, comandi
- `ProjectViewModel` - wrapper progetto per binding
- `FileTreeItemViewModel` - nodo albero file con expand/collapse

**Views:**
- `MainWindow.axaml` - layout completo con sidebar, tabs, terminal panel
- `FileTreeView.axaml` - TreeView per navigazione .personal

**Converters:**
- `PriorityConverters.cs` - colori badge priority
- `StringEqualsConverter.cs` - comparazione per tab attivo

**Build:** Compilazione OK, 0 errori, 0 warning

## Next Steps

### Short Term (Prossima sessione)

1. **Markdown Rendering** (feature prioritaria)
   - Aggiungere Markdown.Avalonia package
   - Implementare toggle view (view/edit)
   - Basic rendering per file .personal/

2. **Issue Management UI** (alta priorita)
   - Parser frontmatter YAML (YamlDotNet)
   - Kanban board base (4 colonne)
   - Drag & drop per status change

3. **Claude Context Panel** (media priorita)
   - Vista unificata config (global, workspace, project)
   - Mostrare CLAUDE.md, .rules/, .personal/ in un'unica vista
   - Effective configuration calculator

### Medium Term

4. **Enhanced Markdown Editor**
   - Split view edit mode
   - Syntax highlighting
   - Link navigation

5. **Advanced Issue Management**
   - Quick metadata editing form
   - Create from template
   - Statistics/filtering

6. **VS Code Extension Exploration**
   - Ricerca su API estensioni VS Code
   - Prototipo minimo con TreeView
   - Valutare integrazione con estensione Claude Code ufficiale

## Current Architecture

### Controls
```
Views/Controls/
├── MultiSidebar.axaml/.cs    - Reusable sidebar system
└── (CollapsibleSidebar)       - Deprecated, sostituito da MultiSidebar
```

### Models
```
Models/
├── Workspace.cs               - Workspace container (HasRules, HasMemoryBank, etc.)
├── AppSettings.cs             - App settings (WorkspaceType)
├── Project.cs                 - Project metadata
├── PersonalFile.cs            - File tree node
├── SidebarItem.cs             - Sidebar definition
└── ...
```

### Converters
```
Converters/
├── BoolToColorConverter.cs    - Bool → colore (verde/grigio) NEW
├── PriorityConverters.cs      - Priority → colore badge
├── StringEqualsConverter.cs   - Comparazione stringhe
├── EnumEqualsConverter.cs     - Comparazione enum
└── EnumNotEqualsConverter.cs  - Negazione enum
```

### Key Patterns
- **MVVM**: Clean separation UI/logic
- **DataTemplate**: Content rendering flexibility
- **RelativeSource binding**: Cross-control DataContext access
- **Command pattern**: All user actions via ICommand

## Blockers / Attention

- No active blockers
- `.personal/` e in `.gitignore` (by design - documentazione locale)

## Feature Files

| Feature | Priority | Status | Location |
|---------|----------|--------|----------|
| Markdown Rendering | High | Planned | `planning/feature-markdown-rendering.md` |
| Issue Management | High | Planned | `planning/feature-issue-management.md` |
| ~~Embedded Terminal~~ | ~~Medium~~ | **Cancelled** | `planning/feature-embedded-terminal.md` (vedi ADR-007) |
| Claude Context Panel | Medium | Planned | `planning/feature-claude-context.md` |

## Quick Links

| What | Where |
|------|-------|
| Main roadmap | `planning/roadmap.md` |
| Feature specs | `planning/feature-*.md` |
| Tech debt | `active/tech-debt-todo.md` |
| Decisions | `reference/decisions/` |
| Analysis reports | `archive/analysis/` |

## Build Status

- Compiles: Yes (0 errors, 0 warnings)
- Runs: Yes
- UI Functional: Yes (sidebar navigation, project selection, workspace detection)
- Data Binding: Yes (all bindings working)
- Features Complete: No (MVP in progress)

## Session Statistics

**Session 2026-01-01:**
- Files modified: Models (2), Services (3), ViewModels (1), Converters (1), Views (1)
- New features: Workspace structure detection, workspace type toggle, status indicators
- Commits: TBD
- Time investment: TBD

**Session 2025-12-30:**
- Files deleted: 7 (terminal-related)
- Files modified: 4 (cleanup)
- ADR created: 1 (ADR-007)

**Session 2025-12-11:**
- Files created: 5 (SidebarItem, MultiSidebar.axaml/.cs, 3 feature specs)
- Files modified: 2 (MainWindow.axaml, MainWindowViewModel.cs)
- Commits: 1 (MultiSidebar implementation)
- Issues resolved: 1 (ContentControl binding)
- Time investment: ~4 ore
- External help: Opus (code review del binding issue)
