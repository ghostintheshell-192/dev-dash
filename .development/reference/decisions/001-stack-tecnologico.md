# ADR-001: Stack tecnologico - C# + Avalonia

**Data**: 2024-12-02 (originale), 2025-12-03 (rivista)
**Status**: Accettata (rivista)

## Contesto

DevDash è un'applicazione desktop che deve:

- Accedere al filesystem locale (progetti, configurazioni, documentazione)
- Leggere/scrivere file markdown
- Potenzialmente lanciare processi esterni (Claude Code)
- Essere usata quotidianamente come strumento di lavoro

Opzioni considerate:

1. **React standalone** + MCP per filesystem
2. **React + Electron** per accesso nativo
3. **React + Tauri** (Rust backend, WebView frontend)
4. **C# + Avalonia** (nativo cross-platform)

## Decisione originale (2024-12-02)

React standalone per prototipazione veloce, con piano di migrare a Electron/Tauri.

## Decisione rivista (2025-12-03)

**C# + Avalonia** come stack definitivo.

## Rationale

### Conoscenza del linguaggio

L'utente conosce C# ma non React. Questo significa:

- Può capire, modificare e debuggare il codice autonomamente
- Non dipende da assistenza AI per ogni modifica
- Curva di apprendimento minima (solo Avalonia, non un nuovo linguaggio)

### Vantaggi tecnici

| Aspetto | C# + Avalonia | React + Electron |
|---------|---------------|------------------|
| Filesystem | `System.IO` nativo | IPC complesso |
| Processi | `Process.Start` | Node child_process via IPC |
| Performance | Nativo | Chromium overhead |
| Bundle size | ~20-50MB | 150MB+ |
| Memoria | Efficiente | Chromium hungry |
| Cross-platform | Avalonia nativo | Electron nativo |

### Il prototipo React non è perso

Il prototipo `devdash-prototype.tsx` ha validato:

- Layout (sidebar collassabile, area principale, terminale)
- Navigazione (workspace → progetti → file)
- Tab system (.personal, docs, issues, ADR, config)
- Interazioni (expand/collapse, selezione, modal)

Questi concetti si traducono direttamente in Avalonia XAML + MVVM.

## Conseguenze

- **Pro**: Codebase comprensibile, manutenibile autonomamente
- **Pro**: Accesso filesystem semplice e diretto
- **Pro**: Possibilità di distribuzione cross-platform (Linux, Windows, macOS)
- **Contro**: Prototipo da reimplementare (ma logica minima, solo UI mock)
- **Mitigazione**: MVVM separa bene UI da logica, facilitando test e manutenzione

## Note tecniche

### Avalonia

- Framework UI cross-platform per .NET
- XAML simile a WPF
- Pattern MVVM consigliato
- Community Toolkit disponibile per boilerplate ridotto
- Hot reload in sviluppo

### Struttura progetto consigliata

```
DevDash/
├── DevDash.sln
├── src/
│   └── DevDash/
│       ├── App.axaml
│       ├── Models/
│       ├── ViewModels/
│       ├── Views/
│       └── Services/
└── docs/
```
