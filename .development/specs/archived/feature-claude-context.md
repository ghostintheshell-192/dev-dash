# Feature: Claude Context Panel

*Created: 2025-12-25*
*Updated: 2026-01-01*

## Overview

DevDash visualizza il contesto completo che Claude Code vedra quando apre un progetto, mostrando configurazioni a tre livelli (Global, Workspace, Project) in modalita read-only per le config native Claude e read/write per la documentazione.

## Motivation

L'utente ha configurazioni Claude distribuite su piu livelli:

- `~/.claude/` — profilo utente, agents, commands
- `{workspace}/.rules/` — coding standards, workflows, preferences (hidden folder)
- `{workspace}/.memory-bank/` — memoria operativa, handoff sessioni
- `{project}/.claude/` — settings, hooks, MCP

Attualmente non esiste un modo semplice per vedere "cosa Claude sa" quando apre un progetto. DevDash risolve questo problema con una vista unificata.

## Design Principles

1. **Read-only per config Claude native** — settings.json, hooks, MCP non vengono modificati da DevDash
2. **Read/write per documentazione** — .personal/, docs/, CLAUDE.md di progetto
3. **Path configurabili** — nessun path hardcodato, tutto in devdash-config.json
4. **Resilienza** — se un path non esiste, mostra placeholder, non crasha

## User Stories

### US-1: Vedere il contesto globale Claude

**Come** utente DevDash
**Voglio** vedere le mie configurazioni globali Claude (~/.claude/)
**Per** sapere quali agents, commands e preferenze sono attivi globalmente

**Acceptance Criteria:**

- Mostra CLAUDE.md globale (contenuto o preview)
- Lista agents/ con nome e descrizione (prima riga del file)
- Lista commands/ con nome
- Mostra settings.json (formatted JSON viewer)

### US-2: Vedere le regole del workspace

**Come** utente DevDash
**Voglio** vedere le regole del workspace corrente (.rules/)
**Per** capire quali coding standards e workflow sono configurati

**Acceptance Criteria:**

- Mostra stato workspace (indicatori verde/grigio per .rules/, .memory-bank/, CLAUDE.md)
- Mostra bootstrap-type rilevato (coding/writing)
- Mostra goto.yaml parsed (linguaggi configurati)
- Mostra user-preferences.yaml parsed
- Lista coding-standards/*.md
- Lista workflows/*.md
- Questi file SONO editabili da DevDash

### US-3: Vedere la configurazione del progetto

**Come** utente DevDash
**Voglio** vedere la configurazione Claude del progetto selezionato
**Per** capire hooks, permissions, e MCP attivi

**Acceptance Criteria:**

- Mostra CLAUDE.md del progetto (editabile)
- Mostra .claude/settings.json (read-only, formatted)
- Lista hooks configurati con evento e comando
- Lista MCP servers da .mcp.json
- Summary .personal/ (file count, last modified)

### US-4: Path configurabili

**Come** utente DevDash
**Voglio** poter cambiare la posizione delle cartelle di configurazione
**Per** riorganizzare il mio setup senza rompere DevDash

**Acceptance Criteria:**

- devdash-config.json definisce tutti i path
- Path supportano variabili ambiente (%APPDATA%, ~)
- Path supportano alias ({vault}, {workspace})
- Cambiare un path nel config aggiorna immediatamente DevDash

## Technical Design

### Existing Models (Updated)

```csharp
// Workspace.cs - gia implementato
public record Workspace
{
    public required int Id { get; init; }
    public required string Name { get; init; }
    public required string Path { get; init; }
    public required string Type { get; init; }  // "coding" | "writing"
    public required string Icon { get; init; }
    public bool HasRules { get; init; }         // .rules/ presente
    public bool HasMemoryBank { get; init; }    // .memory-bank/ presente
    public bool HasClaudeMd { get; init; }      // CLAUDE.md presente
    public string? BootstrapType { get; init; } // "coding" | "writing" | null
}

// AppSettings.cs - gia implementato
public record AppSettings
{
    public string? WorkspacePath { get; init; }
    public string? ClaudeConfigPath { get; init; }
    public string WorkspaceType { get; init; } = "coding";  // "coding" | "writing"
}
```

### Existing Services (Updated)

```csharp
// IFileSystemService.cs - gia implementato
public interface IFileSystemService
{
    // ... metodi esistenti ...
    PersonalFile? GetMemoryBankTree(string workspacePath);
    PersonalFile? GetRulesTree(string workspacePath);
}

// WorkspaceService.cs - gia implementato
public class WorkspaceService : IWorkspaceService
{
    private string? DetectBootstrapType(string workspacePath)
    {
        var rulesPath = Path.Combine(workspacePath, ".rules");
        if (_fileSystem.FileExists(Path.Combine(rulesPath, "bootstrap-coding.md")))
            return "coding";
        if (_fileSystem.FileExists(Path.Combine(rulesPath, "bootstrap-writing.md")))
            return "writing";
        return null;
    }
}
```

### Existing ViewModel Properties (Updated)

```csharp
// MainWindowViewModel.cs - gia implementato
[ObservableProperty] private string _workspaceType = "coding";
[ObservableProperty] private bool _isCodingWorkspace = true;
[ObservableProperty] private bool _workspaceHasRules;
[ObservableProperty] private bool _workspaceHasMemoryBank;
[ObservableProperty] private bool _workspaceHasClaudeMd;
```

### Existing Converters (Updated)

```csharp
// BoolToColorConverter.cs - gia implementato
// Converte bool in colore: true → #22c55e (verde), false → #64748b (grigio)
```

### New Services (To Implement)

```csharp
public interface IPathResolver
{
    string Resolve(string pathTemplate);
    string GetGlobalClaudePath();
    string GetWorkspaceRulesPath(string workspacePath);  // workspace/.rules/
    string GetMemoryBankPath(string workspacePath);       // workspace/.memory-bank/
    string GetProjectPath(string workspaceName, string projectName);
    string GetVaultPath();
    bool PathExists(string path);
}

public interface IClaudeConfigService
{
    // Global
    ClaudeGlobalConfig GetGlobalConfig();
    IReadOnlyList<AgentInfo> GetAgents();
    IReadOnlyList<CommandInfo> GetCommands();

    // Project
    ClaudeProjectConfig GetProjectConfig(string projectPath);
    IReadOnlyList<McpServer> GetMcpServers(string projectPath);
    IReadOnlyList<HookInfo> GetHooks(string projectPath);

    // Summary
    ClaudeContextSummary GetContextSummary(string projectPath);
}
```

### New Models (To Implement)

```csharp
public record ClaudeGlobalConfig(
    string ClaudeMdContent,
    string? SettingsJson
);

public record AgentInfo(
    string Name,
    string FilePath,
    string? Description  // prima riga del file
);

public record CommandInfo(
    string Name,
    string FilePath
);

public record ClaudeProjectConfig(
    string? ClaudeMdContent,
    string? SettingsJson,
    string? SettingsLocalJson
);

public record McpServer(
    string Name,
    string Type,  // "http" | "stdio"
    string? Url,
    string? Command
);

public record HookInfo(
    string Event,      // "SessionStart", "PreToolUse", etc.
    string Matcher,
    string HookType,   // "command" | "prompt"
    string Command
);

public record ClaudeContextSummary(
    ClaudeGlobalConfig Global,
    WorkspaceConfig Workspace,
    ClaudeProjectConfig Project,
    PersonalFolderSummary PersonalFolder
);
```

### Config File (devdash-config.json)

```json
{
  "paths": {
    "globalClaude": "~/.claude",
    "workspaces": [
      {
        "name": "Coding",
        "path": "D:/repos",
        "type": "coding"
      }
    ],
    "vault": "D:/documenti/Vault@Claude",
    "sessionNotes": "{vault}/progetti"
  },
  "ui": {
    "defaultWorkspace": "Coding",
    "showReadOnlyBadge": true
  }
}
```

### Config File Locations (Search Order)

1. `%APPDATA%/DevDash/devdash-config.json` (Windows)
2. `~/.config/devdash/devdash-config.json` (Linux/macOS)
3. `{executable-dir}/devdash-config.json` (portable mode)

## UI Mockup

```text
+---------------------------------------------------------------+
|  Claude Context                                    [Refresh]  |
+---------------------------------------------------------------+
|                                                               |
|  GLOBAL (~/.claude/)                          [read-only]     |
|  +-----------------------------------------------------------+|
|  | CLAUDE.md    "Profilo cognitivo, preferenze generali..."  ||
|  | settings     model: claude-sonnet-4-...                   ||
|  | agents/      3 files (code-reviewer, security-auditor...) ||
|  | commands/    2 files (/init, /review)                     ||
|  +-----------------------------------------------------------+|
|                                                               |
|  WORKSPACE (Coding)                                           |
|  +-----------------------------------------------------------+|
|  | Status:  ● .rules/  ● .memory-bank/  ● CLAUDE.md          ||
|  | Type:    coding (detected from bootstrap-coding.md)       ||
|  | .rules/                                                   ||
|  |   goto.yaml         5 languages configured                ||
|  |   user-preferences  preflight: enabled                    ||
|  |   coding-standards/ 6 files                   [Edit]      ||
|  |   workflows/        3 files                   [Edit]      ||
|  +-----------------------------------------------------------+|
|                                                               |
|  PROJECT (dev-dash)                           Language: C#   |
|  +-----------------------------------------------------------+|
|  | CLAUDE.md           "DevDash project..."      [Edit]      ||
|  | .claude/settings    hooks: 1, permissions: default        ||
|  | .mcp.json           1 server (claude-vscode)              ||
|  | .personal/          12 files, last: 2025-12-25            ||
|  +-----------------------------------------------------------+|
|                                                               |
|  [View Merged CLAUDE.md]  [View Effective Settings]           |
+---------------------------------------------------------------+
```

**Legenda indicatori stato:**
- ● verde (#22c55e) = presente
- ● grigio (#64748b) = assente

## Implementation Phases

### Phase 1: Foundation (PARTIALLY DONE)

1. ~~Create `Workspace` model with status properties~~ DONE
2. ~~Implement workspace detection in `WorkspaceService`~~ DONE
3. ~~Add workspace status indicators in UI~~ DONE
4. Create `DevDashConfig` model and loader
5. Implement `PathResolver` service
6. Add config file location logic

### Phase 2: Claude Config Reading

7. Implement `ClaudeConfigService` (read-only)
8. Parse settings.json, .mcp.json
9. Extract agents/commands from ~/.claude/

### Phase 3: UI

10. Create `ClaudeContextViewModel`
11. Create `ClaudeContextPanel` view
12. Integrate into main window (new sidebar item or tab)

### Phase 4: Polish

13. Add refresh functionality
14. Handle missing paths gracefully
15. Add "View Merged" dialogs

## Open Questions

1. **Settings tab location**: Nuovo item nella sidebar o tab nel main area?
2. **Merged CLAUDE.md**: Come visualizzare gli @imports risolti?
3. **Real-time updates**: FileSystemWatcher per aggiornamenti automatici?

## Related Documents

- [ARCHITECTURE.md](../../docs/ARCHITECTURE.md) — Overall architecture
- [CLAUDE-CONFIG-DISCOVERY.md](../../docs/CLAUDE-CONFIG-DISCOVERY.md) — Config discovery logic
- [ADR-004](../reference/decisions/004-claude-integration.md) — Decision record (pending)
- [ADR-005](../reference/decisions/005-configurable-paths.md) — Decision record (pending)

## Estimated Effort

- Phase 1 (Foundation): ~1 hour remaining (config file loading)
- Phase 2 (Services): 3-4 hours
- Phase 3 (UI): 4-5 hours
- Phase 4 (Polish): 2-3 hours

**Total MVP**: ~10-12 hours remaining
