# ADR-007: Rimozione del Terminale Embedded

**Data**: 2025-12-30
**Status**: Accettata

## Contesto

Durante lo sviluppo di DevDash, era stata implementata una feature di terminale embedded utilizzando **Pty.Net** per consentire l'esecuzione di Claude Code direttamente dall'applicazione, senza dover passare a finestre esterne.

L'implementazione includeva:
- `TerminalService` con gestione PTY (Pseudo-Terminal)
- `TerminalViewModel` per binding MVVM
- `TerminalView` con UI per input/output
- `AnsiParser` per parsing dei codici ANSI
- Dipendenza da `sch.pty.net` v0.3.36-pre
- Integrazione nel `MainWindow` con panel collapsabile

La feature era funzionante ma non ancora committata (solo file untracked in working directory).

## Problema

Durante una revisione architetturale sono emersi diversi punti critici:

### 1. Sovra-ingegnerizzazione
- **Complessità tecnica elevata**: Gestione PTY cross-platform, parsing ANSI, I/O asincrono, thread safety
- **Duplicazione di funzionalità**: Il terminale di sistema (Windows Terminal, iTerm, ecc.) offre già tutte queste feature in modo più robusto
- **Manutenzione onerosa**: Bug del PTY, edge cases, supporto multi-piattaforma richiederebbero continuo effort

### 2. Esperienza utente limitata
Un terminale embedded custom non potrà mai competere con terminali nativi in termini di:
- History e autocomplete avanzati
- Customizzazione (temi, font, keybindings)
- Integrazione con l'ecosistema (tmux, shell plugins, ecc.)
- Performance e rendering

### 3. Conflitto con la filosofia di DevDash
La filosofia dichiarata del progetto è:
> "DevDash manages documentation and context. Claude Code manages execution and automation."

Il terminale embedded sposta DevDash verso l'esecuzione, quando il vero valore è nella **gestione del contesto e della documentazione**.

### 4. Piano di evoluzione verso VS Code Extension
È emerso che DevDash potrebbe evolvere in un'estensione VS Code (vedi [ADR-006](006-desktop-vs-vscode-extension.md)). In quel contesto:
- VS Code ha già un terminale integrato eccellente
- L'estensione Claude Code ufficiale è disponibile in VS Code
- Un terminale custom sarebbe completamente ridondante

## Decisione

**Rimuovere completamente la feature del terminale embedded da DevDash.**

Azioni concrete:
1. ✅ Eliminare tutti i file del terminale (Services, ViewModels, Views, AnsiParser)
2. ✅ Ripristinare i file modificati allo stato pre-terminale
3. ✅ Rimuovere la dipendenza `Pty.Net` dal `.csproj`
4. ✅ Verificare che il progetto compili senza errori
5. 📝 Documentare la decisione in questo ADR

**Non sostituire con alternative** come bottoni "Open in Terminal", per evitare di lasciare codice che non sarà portato nell'eventuale estensione VS Code.

## Rationale

### Perché rimuovere invece di semplificare?

**Alternative considerate:**

| Opzione | Pro | Contro |
|---------|-----|--------|
| **Bottone "Open in Terminal"** | Semplice, zero complessità | Rimane codice non portabile in VS Code Extension |
| **Quick Commands Panel** | Utile per comandi predefiniti | Ridondante con terminale esterno + Claude Code |
| **Terminale semplificato (Process I/O)** | Più leggero di PTY | Claude Code è interattivo, non funzionerebbe bene |
| **Rimozione completa** ✓ | Codebase pulito, focus chiaro | Nessuno significativo |

La **rimozione completa** è l'unica scelta che:
- Mantiene il codebase focalizzato sul core value (documentazione + contesto)
- Non lascia "debito tecnico" da rimuovere successivamente
- Allinea l'implementazione alla filosofia dichiarata del progetto
- Non introduce codice che sarebbe eliminato nel porting a VS Code Extension

### Valore reale di DevDash

Il valore di DevDash NON è nell'emulazione di un terminale, ma in:
- ✅ Vista unificata della documentazione (`.personal/`, `docs/`, `CLAUDE.md`)
- ✅ Gestione strutturata di bug/issue senza overhead GitHub
- ✅ Context panel che mostra cosa Claude "vede" (config, rules, memory)
- ✅ Quick navigation tra progetti della workspace
- ✅ Issue tracking integrato con la documentazione

Nessuno di questi benefici richiede un terminale embedded.

## Conseguenze

### Pro
- ✅ **Codebase più semplice e manutenibile** - Rimozione di ~600 linee di codice complesso
- ✅ **Focus chiaro** - DevDash rimane uno strumento di documentazione, non di esecuzione
- ✅ **Nessuna dipendenza problematica** - `Pty.Net` era in versione pre-release
- ✅ **Preparazione per VS Code Extension** - Nessun codice da portare/eliminare
- ✅ **Meno superficie di testing** - Non serve testare PTY, ANSI parsing, ecc.

### Contro
- ❌ **Perdita di feature implementata** - Lavoro già svolto viene scartato
  - **Mitigazione**: Il lavoro è stato educativo, ha permesso di esplorare PTY e capire che non era la strada giusta
- ❌ **Nessun lancio rapido di Claude** - Bisogna aprire terminale separato
  - **Mitigazione**: Non è un problema reale - gli utenti hanno già terminali aperti quando sviluppano

### Impatti

**Codice:**
- File eliminati: 7 (TerminalService, TerminalViewModel, TerminalView, AnsiParser, test)
- File ripristinati: 4 (App.axaml.cs, DevDash.csproj, MainWindowViewModel, MainWindow.axaml)
- Dipendenze rimosse: 1 (sch.pty.net)
- Compilazione: ✅ OK (0 errori, 0 warning)

**Documentazione:**
- Feature spec `feature-embedded-terminal.md` marcata come **cancelled**
- Roadmap aggiornata per rimuovere riferimenti al terminale
- `CURRENT-STATUS.md` aggiornato

**UI:**
- Il panel "Terminal" nella sidebar rimane visibile (con placeholder UI)
- Non viene rimosso perché la UI potrebbe essere riutilizzata per altre feature future
- Se si decide di eliminarlo definitivamente, sarà un task separato

## Note per il futuro

### Se serve integrazione con Claude Code

Invece di emulare un terminale, considerare:

**Opzione 1: Deep Links**
```csharp
// Apre Claude Code nel progetto con un comando specifico
Process.Start("claude", $"--resume {sessionId}");
```

**Opzione 2: File System Integration**
- DevDash modifica `CLAUDE.md`, `.personal/`, `rules/`
- Claude Code legge automaticamente i cambiamenti
- Zero comunicazione diretta necessaria

**Opzione 3: MCP Server** (avanzato)
- DevDash espone un MCP server
- Claude Code si connette come client
- Comunicazione bidirezionale via protocol standard

Nessuna di queste opzioni richiede un terminale embedded.

## Related ADRs

- [ADR-006: DevDash Desktop vs VS Code Extension](006-desktop-vs-vscode-extension.md) - Contesto sulla possibile evoluzione verso estensione

## Riferimenti

- Feature spec originale: `.personal/planning/feature-embedded-terminal.md`
- Discussione: Session 2025-12-30 con Claude Sonnet 4.5
- Commit di rimozione: (pending - file non erano ancora committati)
