# Architettura DevDash

## Concetti chiave

### Workspace

Un workspace e un contenitore logico di progetti con caratteristiche comuni:

| Workspace | Path | Tipo contenuto |
|-----------|------|----------------|
| Coding | `/data/repos` | Repository software |
| Writing | `/data/documenti/Vault@Racconti` | Progetti creativi |

I workspace sono configurati nelle impostazioni DevDash. Ogni workspace ha una struttura standard:

```
workspace/
├── CLAUDE.md              # Entry point per Claude Code
├── .rules/                # Standards e bootstrap (hidden folder)
│   ├── bootstrap-coding.md    # oppure bootstrap-writing.md
│   ├── user-preferences.yaml
│   ├── core/
│   ├── workflows/
│   └── coding-standards/
└── .memory-bank/          # Memoria operativa (hidden folder)
    ├── progetti/          # Handoff sessioni (per coding)
    └── sessioni/          # Archivio conversazioni
```

### Workspace Model

Il modello `Workspace.cs` rappresenta un workspace con le seguenti proprieta:

| Proprieta | Tipo | Descrizione |
|-----------|------|-------------|
| `Id` | int | Identificativo univoco |
| `Name` | string | Nome del workspace |
| `Path` | string | Path assoluto |
| `Type` | string | "coding" o "writing" |
| `Icon` | string | Emoji icona |
| `HasRules` | bool | Presenza di `.rules/` |
| `HasMemoryBank` | bool | Presenza di `.memory-bank/` |
| `HasClaudeMd` | bool | Presenza di `CLAUDE.md` |
| `BootstrapType` | string? | Tipo rilevato da `bootstrap-*.md` |

### Progetto

Un progetto vive dentro un workspace e puo avere:

- **`.personal/`** - Documentazione privata per spec-driven development
- **`docs/`** - Documentazione pubblica/ufficiale
- **`.claude/`** - Configurazione Claude Code locale (opzionale)
- **`CLAUDE.md`** - Istruzioni Claude Code a livello progetto

### Struttura .personal

```
.personal/
├── INDEX.md              # Punto di ingresso, overview progetto
├── CURRENT-STATUS.md     # Stato attuale, cosa stavo facendo
├── active/               # Lavoro in corso
│   ├── current-notes.md
│   └── tech-debt/        # Debito tecnico da risolvere
├── specs/                # Specifiche funzionalita
│   ├── planned/          # In roadmap
│   ├── backlog/          # Idee parcheggiate
│   └── completed/        # Archivio
├── reference/            # Materiale di riferimento
│   └── decisions/        # ADR (Architecture Decision Records)
└── business/             # Note marketing, monetizzazione
```

---


## Sistema configurazioni Claude Code

DevDash visualizza e permette di modificare le configurazioni Claude Code distribuite su tre livelli:

### Livello 1: Global (`~/.claude/`)

```
~/.claude/
├── settings.json          # Configurazione Claude Code (schema strict)
├── user-profile.md        # Profilo utente, preferenze comunicazione
├── hooks/                 # Hook globali (session archiving, etc.)
├── agents/                # Agenti riutilizzabili
│   ├── code-reviewer.md
│   ├── security-auditor.md
│   └── api-designer.md
└── commands/              # Comandi slash personalizzati
```

### Livello 2: Workspace (`.rules/` - portabile)

```
workspace/
├── CLAUDE.md              # Entry point, regole workspace-wide
└── .rules/                # Hidden folder, portabile con il workspace
    ├── bootstrap-coding.md    # Bootstrap per sessioni coding
    ├── bootstrap-writing.md   # Bootstrap per sessioni writing
    ├── user-preferences.yaml  # Preferenze workflow
    ├── goto.yaml              # Language detection routing
    ├── core/
    │   ├── principles.md
    │   └── security-boundaries.md
    ├── workflows/
    │   ├── git.md
    │   ├── session.md
    │   └── personal-folder.md
    └── coding-standards/
        ├── general-principles.md
        ├── csharp-dotnet.md
        └── ...
```

### Livello 3: Project (ogni repo)

```
progetto/
├── CLAUDE.md              # Override specifici progetto
└── .claude/
    ├── settings.json      # Settings progetto (read-only)
    └── .mcp.json          # MCP servers configurati
```

### Effective Configuration

Le configurazioni si combinano con precedenza: **Project > Workspace > Global**.

DevDash mostrera la "effective configuration" risultante dal merge dei tre livelli.

---

## Memory Bank

La `.memory-bank/` e la memoria operativa del workspace:

```
.memory-bank/
├── progetti/           # Per workspace coding
│   └── [project].md    # Handoff note per progetto
└── sessioni/           # Archivio conversazioni (auto-saved by hook)
```

Questa cartella e:
- **Hidden** (prefisso `.`) per non inquinare la root
- **Portabile** con il workspace
- **Opzionale** - DevDash funziona anche senza

---

## Integrazione Vault@Claude

Per avere una vista unificata di tutte le configurazioni e documentazione, usiamo symlink verso un vault Obsidian:

```
/data/documenti/Vault@Claude/
├── _sistema/                          # Symlink config Claude
│   ├── global/          → ~/.claude/
│   ├── workspace/       → /data/repos/CLAUDE.md + .rules/
│   └── progetti/
│       ├── sheet-atlas/ → /data/repos/sheet-atlas/.claude/
│       └── ...
│
├── progetti/                          # Symlink .personal di ogni progetto
│   ├── sheet-atlas/     → /data/repos/sheet-atlas/.personal/
│   ├── government-feed/ → /data/repos/government-feed/.personal/
│   └── ...
│
└── _automazione/
    ├── validate-config.py
    └── effective-config.py
```

### Vantaggi

1. **Obsidian come viewer** - Navigazione, ricerca, graph view gratis
2. **MCP integration** - Claude Code/Desktop possono accedere via plugin
3. **Backup centralizzato** - Un vault = tutto il contesto
4. **Zero duplicazione** - Symlink, non copie

### Setup symlink (Linux/macOS)

```bash
# Creare la struttura base
mkdir -p /data/documenti/Vault@Claude/{_sistema/{global,workspace,progetti},progetti}

# Symlink configurazioni
ln -s ~/.claude /data/documenti/Vault@Claude/_sistema/global
ln -s /data/repos/CLAUDE.md /data/documenti/Vault@Claude/_sistema/workspace/
ln -s /data/repos/.rules /data/documenti/Vault@Claude/_sistema/workspace/

# Symlink .personal di ogni progetto
ln -s /data/repos/sheet-atlas/.personal /data/documenti/Vault@Claude/progetti/sheet-atlas
```

---


## Integrazione Claude Code

### Modalita previste

1. **Launch con contesto** - Aprire Claude Code con `.personal/INDEX.md` gia caricato
2. **Issue → Task** - Convertire issue selezionata in prompt Claude Code
3. **Config editing** - Modificare configurazioni e vedere effective config live

### MCP (Model Context Protocol)

Con il plugin [obsidian-claude-code-mcp](https://github.com/iansinnott/obsidian-claude-code-mcp):

- Claude Code auto-discover il vault via WebSocket (porta 22360)
- Claude Desktop accede via HTTP/SSE
- Entrambi possono leggere/scrivere nel vault

Questo permette a DevDash di "comandare" Claude Code indirettamente, modificando file che Claude legge.

---

## DevDash Desktop vs VS Code Extension

DevDash esistera in due versioni con filosofie distinte:

| | Desktop (Avalonia) | Extension (VS Code) |
| --- | --- | --- |
| **Claude Code** | Delegato a terminale esterno | Delegato all'estensione ufficiale |
| **Focus** | Docs + context + workspace management | Solo: docs + context + workflow |
| **Autonomia** | Standalone | Companion di Claude Code Extension |

L'integrazione tra le versioni avviene tramite **filesystem** (CLAUDE.md, .rules/, .personal/), non tramite API. Entrambe leggono/scrivono gli stessi file.

**Ordine di sviluppo**: Desktop prima (validazione workflow), Extension dopo.

Per dettagli completi, vedere [ADR-006](../.personal/reference/decisions/006-desktop-vs-vscode-extension.md).

---

## Decisioni architetturali

Le ADR (Architecture Decision Records) sono documentate in `.personal/reference/decisions/`:

- [001 - Stack tecnologico (C# + Avalonia)](../.personal/reference/decisions/001-stack-tecnologico.md)
- [002 - Symlink vs Copy](../.personal/reference/decisions/002-symlink-vs-copy.md)
- [003 - Issue tracking locale](../.personal/reference/decisions/003-issue-tracking-locale.md)
- [006 - DevDash Desktop vs VS Code Extension](../.personal/reference/decisions/006-desktop-vs-vscode-extension.md)
- [007 - Rimozione Terminale Embedded](../.personal/reference/decisions/007-rimozione-terminale-embedded.md)
