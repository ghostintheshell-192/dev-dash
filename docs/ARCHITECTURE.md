# Architettura DevDash

## Concetti chiave

### Workspace

Un workspace è un contenitore logico di progetti con caratteristiche comuni:

| Workspace | Path | Tipo contenuto |
|-----------|------|----------------|
| Coding | `/data/repos` | Repository software |
| Writing | `/data/documenti/Vault@Racconti` | Progetti creativi |

I workspace sono configurati in `~/.claude/CLAUDE.md` nella sezione `goto.yaml`.

### Progetto

Un progetto vive dentro un workspace e può avere:

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
├── specs/                # Specifiche funzionalità
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
├── CLAUDE.md              # Configurazione principale
├── bootstrap-coding.md    # Istruzioni per sessioni coding
├── commands/              # Comandi slash personalizzati
└── agents/                # Agenti specializzati
    ├── code-reviewer.md
    ├── security-auditor.md
    └── api-designer.md
```

### Livello 2: Workspace (`/data/repos/`)

```
/data/repos/
├── CLAUDE.md              # Regole workspace-wide
└── .claude/
    └── rules/             # Regole modulari
        ├── csharp-conventions.md
        └── git-workflow.md
```

### Livello 3: Project (ogni repo)

```
progetto/
├── CLAUDE.md              # Override specifici progetto
└── .claude/
    └── rules/             # Regole solo questo progetto
```

### Effective Configuration

Le configurazioni si combinano con precedenza: **Project > Workspace > Global**.

DevDash mostrerà la "effective configuration" risultante dal merge dei tre livelli.

---

## Integrazione Vault@Claude

Per avere una vista unificata di tutte le configurazioni e documentazione, usiamo symlink verso un vault Obsidian:

```
/data/documenti/Vault@Claude/
├── _sistema/                          # Symlink config Claude
│   ├── global/          → ~/.claude/
│   ├── workspace/       → /data/repos/CLAUDE.md + .claude/
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
ln -s /data/repos/.claude /data/documenti/Vault@Claude/_sistema/workspace/

# Symlink .personal di ogni progetto
ln -s /data/repos/sheet-atlas/.personal /data/documenti/Vault@Claude/progetti/sheet-atlas
```

---

## Integrazione Claude Code

### Modalità previste

1. **Launch con contesto** - Aprire Claude Code con `.personal/INDEX.md` già caricato
2. **Issue → Task** - Convertire issue selezionata in prompt Claude Code
3. **Config editing** - Modificare configurazioni e vedere effective config live

### MCP (Model Context Protocol)

Con il plugin [obsidian-claude-code-mcp](https://github.com/iansinnott/obsidian-claude-code-mcp):

- Claude Code auto-discover il vault via WebSocket (porta 22360)
- Claude Desktop accede via HTTP/SSE
- Entrambi possono leggere/scrivere nel vault

Questo permette a DevDash di "comandare" Claude Code indirettamente, modificando file che Claude legge.

---

## Decisioni architetturali

Le ADR (Architecture Decision Records) sono documentate in `.personal/reference/decisions/`:

- [001 - Stack tecnologico (C# + Avalonia)](/.personal/reference/decisions/001-stack-tecnologico.md)
- [002 - Symlink vs Copy](/.personal/reference/decisions/002-symlink-vs-copy.md)
- [003 - Issue tracking locale](/.personal/reference/decisions/003-issue-tracking-locale.md)
