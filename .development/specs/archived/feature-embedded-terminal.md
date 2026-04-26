---
type: feature
priority: medium
status: cancelled
category: integration
related: [ADR-007]
cancelled_date: 2025-12-30
cancelled_reason: "Sovra-ingegnerizzazione - Terminale esterno è sufficiente, focus su documentazione"
---

# Embedded Claude Terminal

> **⚠️ FEATURE CANCELLATA**
>
> Questa feature è stata cancellata il 2025-12-30 dopo revisione architetturale.
>
> **Motivazione**: Il terminale embedded aggiungeva complessità eccessiva (gestione PTY, ANSI parsing, I/O asincrono) senza portare valore proporzionale. Il focus di DevDash è la gestione della documentazione e del contesto, non l'esecuzione di comandi. L'uso del terminale di sistema (Windows Terminal, iTerm, ecc.) è più appropriato.
>
> Inoltre, il piano di evoluzione verso VS Code Extension renderebbe il terminale completamente ridondante (VS Code ha già un terminale integrato eccellente).
>
> **Decisione documentata in**: [ADR-007: Rimozione del Terminale Embedded](../reference/decisions/007-rimozione-terminale-embedded.md)
>
> Il documento originale è mantenuto per riferimento storico.

## Overview

Integrare un terminale dentro DevDash che consenta di eseguire Claude Code direttamente dall'applicazione, contestualizzato sul progetto selezionato.

## User Story

Come utente, voglio:
- Aprire un terminale Claude Code direttamente da DevDash
- Eseguire comandi Claude nel contesto del progetto selezionato
- Vedere l'output in tempo reale
- Inviare input/comandi a Claude
- Non dover uscire da DevDash per usare Claude

## Technical Approach

### Option 1: Process I/O Capture (Recommended for MVP)

**Implementation:**
```csharp
// TerminalViewModel.cs
private Process? _claudeProcess;

public void StartClaude(string projectPath)
{
    _claudeProcess = new Process
    {
        StartInfo = new ProcessStartInfo
        {
            FileName = "claude",
            WorkingDirectory = projectPath,
            RedirectStandardInput = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true
        }
    };

    _claudeProcess.OutputDataReceived += OnOutputReceived;
    _claudeProcess.ErrorDataReceived += OnErrorReceived;
    _claudeProcess.Start();
    _claudeProcess.BeginOutputReadLine();
    _claudeProcess.BeginErrorReadLine();
}

public void SendCommand(string command)
{
    _claudeProcess?.StandardInput.WriteLine(command);
}
```

**UI:**
```xml
<Grid RowDefinitions="*,Auto">
    <!-- Output area -->
    <ScrollViewer Grid.Row="0">
        <TextBlock Text="{Binding Output}"
                   FontFamily="Consolas,monospace"
                   Background="#0f172a"/>
    </ScrollViewer>

    <!-- Input area -->
    <TextBox Grid.Row="1"
             Text="{Binding InputCommand}"
             Watermark="Type command and press Enter..."/>
</Grid>
```

**Pros:**
- Relativamente semplice da implementare
- Nessuna dipendenza esterna pesante
- Controllo completo su I/O
- Funziona con Claude Code esistente

**Cons:**
- Non supporta ANSI colors nativamente (bisogna parsarli)
- Meno "terminale-like" come esperienza
- Input limitato (no autocomplete, history, ecc.)

### Option 2: PTY-based Terminal Emulator

**Libraries:**
- `Pty.Net` - Cross-platform pseudo-terminal
- `AvaloniaEdit` - Editor component (opzionale)

**Pros:**
- Esperienza terminale completa
- Supporto ANSI colors nativo
- Input avanzato (history, tab completion)

**Cons:**
- Più complesso da implementare
- Dipendenze esterne
- Gestione PTY cross-platform può essere tricky

### Option 3: WebView + xterm.js

**Libraries:**
- `Avalonia.WebView2` (Windows) o `WebViewCore` (cross-platform)
- `xterm.js` per rendering terminale

**Pros:**
- Terminale bellissimo e completo
- Supporto ANSI completo
- Addon disponibili (search, links, ecc.)

**Cons:**
- Dipendenza pesante (WebView)
- Comunicazione JS ↔ C# via bridge
- Overhead di memoria

## Recommended Implementation Plan

### Phase 1 (MVP): Process I/O Capture
- Implementare cattura stdout/stderr
- TextBox per input con history (frecce su/giù)
- Basic ANSI color parsing (optional)
- Auto-scroll su nuovo output

### Phase 2: Enhanced Experience
- Aggiungere PTY support per terminale completo
- Syntax highlighting per output
- Search nel terminale
- Save/export session

### Phase 3: Advanced Features
- Multiple terminal tabs
- Terminal split view
- Custom keybindings
- Integration con project context (auto-cd al progetto)

## UI/UX Considerations

**Placement:**
- Pannello in basso (come in VS Code)
- Collapsibile con resize handle
- Button nella topbar "Open Terminal"

**Context:**
- Auto-imposta working directory sul progetto selezionato
- Passa variabili d'ambiente (PROJECT_PATH, ecc.)
- Chiude/riavvia quando cambio progetto (con conferma)

**Commands:**
- Shortcut: Ctrl+` per toggle terminal
- Click destro su progetto → "Open in Terminal"

## Dependencies

```bash
# Phase 1
dotnet add package System.Diagnostics.Process  # Built-in

# Phase 2 (optional)
dotnet add package Pty.Net
dotnet add package AvaloniaEdit

# Phase 3 (optional)
dotnet add package WebViewCore.Avalonia
```

## Open Questions

1. Come gestire input interattivo (es. Claude chiede conferma)?
2. Preservare la sessione quando cambio progetto?
3. Supportare multiple terminal instances?
4. Logging dell'output in file?

## Success Criteria

- [ ] Posso lanciare `claude` da DevDash
- [ ] Vedo l'output di Claude in tempo reale
- [ ] Posso inviare comandi a Claude
- [ ] Il terminale si apre nella directory corretta del progetto
- [ ] L'output è leggibile (colori opzionali)
- [ ] Posso chiudere/riaprire il terminale

## Timeline Estimate

- **Phase 1 (MVP)**: 1-2 giorni di sviluppo
- **Phase 2**: +2-3 giorni
- **Phase 3**: +3-4 giorni

## Related

- Feature: Markdown Editing (possibile integrazione con Claude per editing assistito)
- Feature: Issue Management (Claude potrebbe aiutare a creare/gestire issue)
