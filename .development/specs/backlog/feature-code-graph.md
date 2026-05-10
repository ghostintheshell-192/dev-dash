---
type: feature
priority: medium
status: planned
category: ui
related: []
---

# Code Graph — Dual-purpose codebase map

*Created: 2026-04-26 — Skeleton ibernato in `planned/` in attesa del consolidamento del nuovo stack.*

> **Stack disclaimer (2026-04-27)**: i riferimenti implementativi a .NET/Roslyn e a `LiveMarkdown.Avalonia` precedono la decisione di pivot a C++/ImGui (vedi `legacy/avalonia-final` e ADR di pivot futuro). I **principi** (single source of truth → due viste, una visuale e una testuale) reggono al pivot; i **dettagli implementativi** (parser, motore di rendering, formati intermedi) sono TBD e vanno riscritti quando il nuovo stack è consolidato.

## Overview

Una rappresentazione **a grafo** della codebase, mantenuta come singola struttura dati e renderizzata in due viste con due consumatori diversi:

- **Vista visuale e interattiva** (per la programmatrice): grafo navigabile dentro dev-dash, con notazione UML class diagram per il livello classe e C4-style per il livello modulo.
- **Vista testuale strutturata** (per Claude): serializzazione tipo "nodo + archi entranti/uscenti + metadata", iniettata nel context al SessionStart per ridurre l'esplorazione grep-first.

Entrambe le viste leggono dalla **stessa struttura dati** (es. JSON), rigenerata da hook su build/commit. Stessa sorgente, due consumatori.

## Motivation

Emerge dalla discussione del [resource-model](../../reference/technical/resource-model.md) (sessione 2026-04-26):

- **`ARCHITECTURE.md` attuale è un albero del filesystem**: utile come orientation, ma non cattura le **relazioni** fra unità di codice (chi dipende da chi, chi usa chi, chi implementa cosa).
- **Per Claude**: il pattern di esplorazione default è grep-first, non map-first. Una mappa di sole gerarchie file non sposta l'ago. Un grafo con relazioni `depends-on` / `used-by` lo sposta — può sostituire molteplici grep mirati con una singola lettura strutturata.
- **Per la programmatrice**: leggere cluster, simmetrie, isole disconnesse, e centralità di certi nodi è naturale visualmente, scomodo testualmente.

L'unione delle due esigenze in un singolo modello dati con due viste è l'intuizione di base.

## Architectural design

### Single source of truth

Una struttura dati JSON (o equivalente) descrive il grafo:

```jsonc
{
  "nodes": [
    {
      "id": "ScaffoldService",
      "kind": "class",
      "path": "src/DevDash/Services/ScaffoldService.cs",
      "module": "Services",
      "summary": "Resolves and applies project scaffolding from templates",
      "lastModified": "2026-04-15"
    }
  ],
  "edges": [
    {
      "from": "ScaffoldService",
      "to": "IFileSystemService",
      "kind": "depends-on"
    },
    {
      "from": "ScaffoldService",
      "to": "IScaffoldService",
      "kind": "implements"
    }
  ]
}
```

Tipi di edge candidati: `depends-on`, `implements`, `extends`, `composes`, `uses-method`, `used-by` (computato come inverso). Da decidere quali sono utili abbastanza da catturare e quali sono rumore.

### Vista visuale

- **Livello classe**: notazione UML class diagram (box con name, eventualmente metodi pubblici principali; freccia con triangolo per `extends`/`implements`; diamante per `composes`; freccia semplice per `depends-on`/`uses`). Multiplicities, role names, attributi privati: omessi by default. Pragmatico, non puristico.
- **Livello modulo**: C4-style (box rettangolari con tagline + frecce con label di intent). UML component diagram esiste ma è raramente usato; C4 è più leggibile a zoom-out.
- **Rendering**: probabilmente Mermaid (sintassi `classDiagram` per UML, sintassi `flowchart` o custom per C4) — già supportato in dev-dash via LiveMarkdown.Avalonia, possibile riuso. Da valutare alternativa con grafica nativa (D3-like force-directed) se l'interattività richiesta supera quello che Mermaid offre.

### Vista testuale (per Claude)

Serializzazione del grafo come adjacency list strutturata in markdown:

```markdown
## ScaffoldService.cs (src/DevDash/Services/, module: Services)
**Summary**: Resolves and applies project scaffolding from templates
**Implements**: IScaffoldService
**Depends on**: IFileSystemService, IPathResolver
**Used by**: WorkspaceService, MainWindowViewModel
**Last modified**: 2026-04-15
```

Iniettata nel context tramite `@import` in `.claude/CLAUDE.md` (stesso meccanismo già usato per `ARCHITECTURE.md`). Da valutare: dimensione complessiva del context speso, soglia oltre la quale tagliare per modulo invece di full graph.

In alternativa: il grafo Mermaid `classDiagram` *è già testo strutturato* — Claude lo legge come tale, non serve una serializzazione custom. Da valutare quale formato dà migliore signal-to-token.

## Generation pipeline

- **Quando**: hook su pre-commit o post-build (vedi pattern già usati per `INDEX.md` e `ARCHITECTURE.md`).
- **Come**: parser del codice C# (Roslyn? regex? alternativa più leggera?). Da decidere: usare un parser AST vero o approssimazioni più semplici (es. analisi di `using`, `:` in class declarations, ecc.). Un parser AST è preciso ma pesante; un parser euristico è leggero ma fragile.
- **Cosa**: estrae nodi (classi, interface, eventualmente moduli) ed edge (depends-on, implements, extends, composes). Annota metadata (path, module, last-modified).

## User stories

- **US-1 (programmatrice)**: aprire il grafo del progetto in dev-dash, navigare visualmente fra classi e moduli, identificare cluster e accoppiamento eccessivo.
- **US-2 (Claude)**: a sessione iniziata, avere il grafo testuale già nel context — quando devo "trovare dove vive `X`" o "capire chi usa `Y`", consulto il grafo invece di fare grep.
- **US-3 (manutenzione)**: il grafo si aggiorna da solo (hook), non c'è da editarlo a mano. Se va out-of-sync col codice, è un bug del generatore.

## Implementation phases (proposta iniziale)

1. **Phase 1 — Data model + parser MVP**: definire schema JSON, scrivere parser euristico che estrae 3-4 tipi di edge essenziali (depends-on, implements, extends). Output: file JSON in `.development/architecture/code-graph.json`.
2. **Phase 2 — Text serialization for Claude**: generare il markdown adjacency list da JSON, esporre come `.development/ARCHITECTURE-graph.md`, importare nel context via `@import`.
3. **Phase 3 — Visual rendering in dev-dash**: vista interattiva. Mermaid first, valutare upgrade.
4. **Phase 4 — Refinements**: edge types aggiuntivi, drill-down, filtri, layout migliori.

Phase 2 è quella con il rapporto valore/complessità migliore secondo me — mi dà già la riduzione di grep esplorativi senza dover implementare la UI.

## Open questions

- **OQ-1**: parser AST (Roslyn) vs parser euristico. Quale soglia di precisione serve davvero per essere utili? Un parser euristico che cattura il 90% degli edge corretti è probabilmente sufficiente, e molto più leggero da costruire e mantenere.
- **OQ-2**: granularità nodi. Solo classi/interface? Anche metodi pubblici come sotto-nodi? Anche moduli/namespace come nodi composti? Più granularità = più valore ma più rumore.
- **OQ-3**: come gestire freschezza. Hook su pre-commit garantisce aggiornamento al commit, ma se la programmatrice sta lavorando in working tree senza commit, il grafo è stale. Vale la pena un "rigenera al volo" trigger? O accettiamo la staleness as-is fra un commit e l'altro?
- **OQ-4**: dimensione del context speso. Se il grafo è 20KB di markdown, è OK iniettarlo always-on come `ARCHITECTURE.md`? O serve qualche meccanismo di lazy loading per progetti grandi (Claude Code lo supporta? — verificare path-scoped rules come pattern adottabile).
- **OQ-5**: integrazione con resource-model. Il code-graph è un caso speciale di "risorsa con due render"? O è una bestia a parte? Pensiero: probabilmente è un caso speciale di **`history-render`** (vedi sezione L del resource-model)? No, history-render è per cronologie temporali. Code-graph è un sesto pattern. Decidere se aggiungerlo.

## Related

- [resource-model.md](../../reference/technical/resource-model.md) — discussione che ha generato questa spec
- `ARCHITECTURE.md` — l'attuale albero filesystem auto-generato, da affiancare (non sostituire) col code-graph

## Estimated effort

Da definire dopo aver risposto alle OQ-1 e OQ-4. La phase 2 (text-only per Claude) è probabilmente nell'ordine di mezza giornata di lavoro se il parser è euristico. La phase 3 (UI visuale) è almeno alcuni giorni, dipendentemente da quanto interattività vogliamo.
