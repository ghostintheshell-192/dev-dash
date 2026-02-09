# Claude Code Configuration Discovery

How DevDash finds and interacts with Claude Code configuration.

---

## Configuration Levels

Claude Code uses a three-level configuration hierarchy:

| Level | Location | Purpose |
|-------|----------|---------|
| **Global** | `~/.claude/` | User-wide settings, user profile, hooks |
| **Workspace** | `.rules/` | Workspace standards and bootstrap |
| **Project** | `.claude/` | Per-project overrides |

---

## Finding Global Configuration

### Case 1: Standalone (Claude Code CLI)

```
Windows:  %USERPROFILE%\.claude\
Linux:    ~/.claude/
macOS:    ~/.claude/
```

**Detection logic:**
```csharp
string GetGlobalConfigPath()
{
    if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        return Path.Combine(Environment.GetFolderPath(
            Environment.SpecialFolder.UserProfile), ".claude");
    else
        return Path.Combine(Environment.GetEnvironmentVariable("HOME"), ".claude");
}
```

### Case 2: VS Code Extension

The Claude Code VS Code extension stores its configuration in a different location:

```
Windows:  %USERPROFILE%\.vscode\extensions\anthropics.claude-code-*\
Linux:    ~/.vscode/extensions/anthropics.claude-code-*/
macOS:    ~/.vscode/extensions/anthropics.claude-code-*/
```

**Note:** VS Code extension may use different config storage. This needs verification.

---

## Global Configuration Contents

What belongs in global config (cross-workspace):

| File | Purpose |
|------|---------|
| `settings.json` | Claude Code settings (strict schema, no custom fields) |
| `user-profile.md` | User cognitive style, communication preferences |
| `hooks/` | System-wide hooks (e.g., session archiving) |
| `agents/` | Reusable custom agents |

### User Profile (user-profile.md)

Since `settings.json` has a strict schema, user information goes in a separate markdown file:

```markdown
# Profilo Utente - [Name]

## Cognitivamente
- Stile di pensiero
- Neurodivergenze
- Cosa funziona/non funziona

## Comunicativamente
- Come rispondere
- Validazione vs sfida
- Leggere tra le righe

## Motivazionalmente
- Driver principali
- Pattern da conoscere

## Relazionalmente
- Stile di connessione
- Cosa evitare
```

The bootstrap file in each workspace should reference this and include a summary.

---

## Workspace Configuration

Workspace config lives in `.rules/` (portable with the workspace):

| File | Purpose |
|------|---------|
| `bootstrap-coding.md` | Essential context for coding sessions |
| `bootstrap-writing.md` | Essential context for writing sessions |
| `user-preferences.yaml` | Workflow preferences |
| `core/` | Principles, security boundaries |
| `workflows/` | Git, sessions, personal folder |
| `coding-standards/` | Language-specific standards |

---

## Memory Bank

Each workspace has a `.memory-bank/` folder for operational memory:

```
.memory-bank/
├── progetti/           # For coding workspaces
│   └── [project].md    # Session handoff per project
└── sessioni/           # Conversation archives (auto-saved by hook)
```

---

## DevDash Workspace Detection

DevDash automatically detects the workspace structure using the `WorkspaceService`:

### Workspace Model Properties

| Property | Type | Description |
|----------|------|-------------|
| `HasRules` | bool | `.rules/` directory exists |
| `HasMemoryBank` | bool | `.memory-bank/` directory exists |
| `HasClaudeMd` | bool | `CLAUDE.md` file exists |
| `BootstrapType` | string? | "coding" or "writing" (detected from bootstrap-*.md) |

### Detection Logic

```csharp
// WorkspaceService.cs
private string? DetectBootstrapType(string workspacePath)
{
    var rulesPath = Path.Combine(workspacePath, ".rules");
    if (!_fileSystem.DirectoryExists(rulesPath))
        return null;

    if (_fileSystem.FileExists(Path.Combine(rulesPath, "bootstrap-coding.md")))
        return "coding";
    if (_fileSystem.FileExists(Path.Combine(rulesPath, "bootstrap-writing.md")))
        return "writing";

    return null;
}
```

### UI Indicators

The settings panel shows workspace status with colored indicators:
- Green (#22c55e) = present
- Gray (#64748b) = absent

Uses `BoolToColorConverter` for visual feedback.

---

## DevDash Initialization Workflow

When user configures a new workspace in DevDash:

1. **User selects folder** - marks as "coding" or "writing" workspace
2. **DevDash creates structure**:
   ```
   workspace/
   ├── CLAUDE.md                    # Entry point
   ├── .memory-bank/
   │   └── progetti/                # or racconti/ for writing
   └── .rules/
       └── bootstrap-{type}.md      # coding or writing
   ```
3. **DevDash discovers global config** - reads user-profile.md
4. **DevDash incorporates user profile** - into bootstrap file

---

## IPathResolver Interface

DevDash should use a configurable path resolver:

```csharp
public interface IPathResolver
{
    string GetGlobalConfigPath();
    string GetUserProfilePath();           // ~/.claude/user-profile.md
    string GetWorkspaceRulesPath(string workspacePath);  // workspace/.rules/
    string GetMemoryBankPath(string workspacePath);      // workspace/.memory-bank/
    string GetProjectConfigPath(string projectPath);
    bool IsVSCodeExtensionMode();
}
```

---

## TODO

- [ ] Verify VS Code extension config storage location
- [x] Implement detection logic in DevDash (WorkspaceService.DetectBootstrapType)
- [ ] Handle case where Claude Code is not installed
- [ ] Support multiple Claude installations (CLI + VS Code)
- [ ] Parse user-profile.md and display in UI
