# DevDash

Dashboard personale per sviluppo in solitaria con Claude Code.

## Cos'è

DevDash è una interfaccia unificata per gestire progetti software e creativi, progettata per chi sviluppa da solo con l'assistenza di AI. Integra:

- **Navigazione progetti** attraverso workspace tematici
- **Gestione documentazione** (.personal + docs)
- **Issue tracking** personale senza overhead di GitHub Issues
- **Configurazioni Claude Code** visualizzate e modificabili
- **Sessioni Claude Code** lanciate con contesto pre-caricato

## Perché esiste

Quando lavori su più progetti usando spec-driven development e mantieni documentazione strutturata, hai bisogno di:

1. Vedere tutto in un posto solo
2. Navigare velocemente tra progetti e loro documentazione
3. Tenere traccia di issue personali che non meritano GitHub
4. Gestire le configurazioni Claude Code sparse su tre livelli
5. Lanciare Claude Code con il contesto giusto già caricato

DevDash risolve questi problemi senza reinventare l'editor (usi comunque VS Code/Obsidian per modificare).

## Stack

- **Framework**: Avalonia UI 11.3 (cross-platform)
- **Language**: C# / .NET 8
- **Pattern**: MVVM (CommunityToolkit.Mvvm)
- **Theme**: Fluent (dark mode)

## Status

🟡 **Prototipo** - UI funzionale con dati mock, non ancora collegata al filesystem reale.

## Prossimi passi

Piano di sviluppo in `.personal/planning/roadmap.md` (6 fasi dalla prototipazione al polish).

## Struttura

```
dev-dash/
├── src/DevDash/             # Applicazione Avalonia
│   ├── Models/              # Data models
│   ├── ViewModels/          # MVVM ViewModels
│   ├── Views/               # XAML views
│   └── Services/            # Business logic
├── devdash-prototype.tsx    # Prototipo UI (reference)
├── docs/
│   ├── ARCHITECTURE.md      # Architettura overview
│   └── SETUP.md             # Guida installazione
└── .personal/               # Documentazione privata (non committata)
    ├── planning/            # Roadmap, piani
    ├── reference/decisions/ # ADR
    └── active/              # Lavoro in corso
```
