# ADR-006: DevDash Desktop vs VS Code Extension

**Data**: 2025-12-25
**Status**: Accepted
**Impact**: high
**Sommario**: Tratta DevDash Desktop ed eventuale estensione VS Code come due prodotti separati con filosofie distinte (desktop standalone vs companion dell'estensione Claude Code), integrati via filesystem e da sviluppare prima Desktop poi Extension; un addendum 2026-06-10 rimanda la ri-decisione sulla forma di distribuzione al verificarsi di un trigger concreto.

## Contesto

DevDash nasce come applicazione desktop per gestire documentazione, spec-driven development, e integrazione con Claude Code. Durante lo sviluppo è emersa la possibilità di creare anche un'estensione VS Code per gli utenti che preferiscono rimanere nell'IDE.

La domanda chiave: sono due prodotti diversi o uno solo con due interfacce?

## Decisione

**Due prodotti separati con filosofie distinte:**

| | DevDash Desktop (Avalonia) | DevDash Extension (VS Code) |
|---|---|---|
| **Claude Code** | ~~Embedded (terminale integrato)~~ **Delegato al terminale esterno** (vedi ADR-007) | Delegato all'estensione ufficiale |
| **Configurazione Claude** | Gestita interamente da DevDash | Gestita dall'estensione ufficiale |
| **Focus** | ~~Tutto: terminal + docs + config~~ **Docs + context + workflow** | Solo: docs + context + workflow |
| **Linguaggio** | C# | TypeScript |
| **Autonomia** | Standalone, indipendente | Companion dell'estensione Claude Code |

## Rationale

### L'integrazione avviene tramite filesystem, non API

Le personalizzazioni Claude Code (hooks, CLAUDE.md, rules/, .personal/) sono **file su disco**. Entrambe le versioni di DevDash possono leggerli e scriverli. L'estensione Claude Code per VS Code li legge automaticamente.

Questo significa che:
- DevDash Extension può gestire tutta la documentazione e configurazione
- L'estensione Claude Code esegue i comandi usando quei file
- Non serve comunicazione diretta tra le due estensioni

### Separazione delle responsabilità

**DevDash Desktop**: ~~controllo totale, esperienza unificata. L'utente non esce mai dall'applicazione.~~ **Focus su documentazione e contesto**. L'esecuzione di Claude Code avviene nel terminale esterno (vedi ADR-007).

**DevDash Extension**: complementare a Claude Code. L'utente ha:
- Claude Code Extension per chat/esecuzione
- DevDash Extension per documentazione/context management

Questa separazione rende l'estensione più snella e focalizzata.

### Ordine di sviluppo

1. **Prima Desktop** - È l'ambiente di lavoro principale, permette di validare il workflow
2. **Poi Extension** - Quando il workflow è consolidato, si porta l'esperienza su VS Code

Non serve svilupparle in parallelo. La logica di business (parsing markdown, gestione frontmatter, struttura rules/) può essere estratta e riscritta in TypeScript per l'estensione.

## Conseguenze

### Pro
- Desktop rimane autonomo e completo
- Extension è leggera, fa una cosa sola bene
- Workflow validato su Desktop prima di portarlo su VS Code
- Meno complessità: non serve far comunicare due estensioni

### Contro
- Due codebase separate (C# e TypeScript)
- Feature parity da mantenere manualmente
- Utenti devono scegliere quale usare

### Mitigazioni
- Documentare bene il workflow in modo che sia replicabile
- Struttura file identica (.personal/, rules/, CLAUDE.md) garantisce compatibilità
- L'estensione può essere sviluppata dopo che Desktop è stabile

## Note per l'implementazione

### DevDash Desktop
- ~~Terminale embedded via Process I/O o PTY~~ **Rimosso - vedi ADR-007**
- Gestione completa di tutti i file Claude (documentazione, configurazione)
- UI completa con Kanban, markdown editor, context panel
- Esecuzione Claude Code delegata al terminale esterno

### DevDash Extension (futuro)
- TreeView per navigare .personal/, specs/, rules/
- WebView per Kanban e markdown editing
- FileSystemWatcher per reagire ai cambiamenti
- **Non** include terminale - usa l'estensione Claude Code ufficiale

### File condivisi (entrambe le versioni)
```
workspace/
├── CLAUDE.md              # Read/Write
├── rules/                 # Read/Write
│   ├── workflows/
│   └── coding-standards/
├── .personal/             # Read/Write
│   ├── specs/
│   └── planning/
└── .claude/               # Read-only (gestito da Claude Code)
```

---

## Addendum 2026-06-10: trigger di ri-decisione sulla forma di distribuzione

Rilettura post-pivot (lo "C#" della tabella sopra è oggi C++20/ImGui, ADR-008)
e post-wedge, alla luce dell'ambizione di distribuzione emersa nella revisione
della visione (vedi ADR-011).

La tensione: per la distribuzione, un'estensione VS Code avrebbe vantaggi
strutturali (marketplace, niente problema di install, Claude Code già "vive"
in VS Code con la sua estensione — niente da integrare). Per l'esplorazione,
il desktop nativo ha libertà che un'estensione non ha (filesystem pieno,
processi, renderer proprio) — e DevDash è anche il laboratorio del filone
"progetto che migliora se stesso".

**Decisione: non si ri-decide ora.** Rideciderlo oggi significherebbe farlo
senza informazioni nuove rispetto al 2025-12. La questione si riapre — con un
ADR dedicato — al verificarsi di un trigger concreto:

- primo utente esterno reale interessato a usare DevDash, oppure
- intenzione concreta di pubblicare (store, marketplace, release pubblica).

Fino ad allora il desktop resta la forma del laboratorio. Cosa resta vero in
entrambi gli esiti (ed è quindi investimento sicuro): il resource model, la
semantica della effective config, scaffold/snapshot come concetti, le spec e
gli ADR — l'integrazione è filesystem-based, la conoscenza di dominio è
identica nelle due forme. Cosa non si trasferisce: la UI ImGui — che è anche
la parte oggi dichiaratamente provvisoria ("tutta da rifare", 2026-05-14).

Resta confermato ADR-007: nessun terminale embedded in nessuna delle due
forme. Il modello a due finestre (Claude nel terminale + DevDash) è validato
dall'uso reale quotidiano.
