---
type: feature
priority: must-have
status: planned
category: core
part_of: wedge
related: [feature-scaffold-management, feature-snapshot-history]
depends_on: []
created: 2026-05-10
---

# Effective Config View

> **Parte 1/3 della wedge feature di DevDash.** Mostra la configurazione
> Claude effettiva (merge global + workspace + project) per il progetto
> corrente, con lineage. Le altre due parti sono
> `feature-scaffold-management.md` (gestione scaffold dell'utente) e
> `feature-snapshot-history.md` (snapshot e restore). Framing strategico:
> `.personal/business/analysis/analysis.md`.

## Summary

Una vista che risolve e visualizza la configurazione Claude Code che sarà
*effettivamente* attiva per il progetto corrente, prendendo in input i
livelli che la compongono (globale → workspace → project) e mostrando la
provenienza di ogni regola/file/setting.

L'analogia di riferimento è `kubectl config view --merged`,
`eslint --print-config`, `tsc --showConfig`: prendi una pila di config
sovrapposte, le risolvi, mostri il risultato finale + da dove viene ogni
pezzo.

## Background

Il framing strategico vive in `.personal/business/analysis/analysis.md`:
nessun tool oggi mostra la configurazione effettiva su base
global/workspace/project nel modo in cui i tool sopra la mostrano per i
loro domini. Questa è la casella vuota nel mercato. Storicamente l'idea
era stata abbozzata in `archived/feature-claude-context.md` (dicembre 2025)
come "vista a tre livelli affiancati"; questa è l'evoluzione: da
*affiancata e statica* a *risolta con lineage*.

## User Stories

### US-1 — Vedere cosa Claude vede oggi

**Come** utente che apre un progetto in DevDash
**Voglio** vedere la *effective configuration* che Claude userà al prossimo
session-start su questo progetto
**Per** capire quali regole, hooks, agents, settings, MCP server sono
attivi e da quale livello vengono.

**Acceptance:**

- La vista mostra il merge dei tre livelli (global, workspace, project)
  con un badge di provenienza per ogni regola/file/setting.
- Funziona anche su progetti senza `.claude/` (mostra "ereditato da
  globale" / "non configurato").
- Funziona anche su progetti con configurazione fatta a mano, senza scaffold.

### US-2 — Vedere config Claude prima di fare nulla

**Come** utente curioso o in fase di valutazione
**Voglio** poter aprire un progetto e vedere la sua config Claude **senza
prima dover applicare uno scaffold o configurare alcunché**
**Per** non essere costretto a un onboarding prima di poter ispezionare.

**Acceptance:**

- US-1 funziona da subito, anche su progetti vergini.
- Le feature di scaffold management (vedi spec separata) e snapshot (idem)
  sono opzionali, mai prerequisito di US-1.

## Requirements

### Funzionali

- [ ] **Resolver di config**: legge `~/.claude/`, eventuale workspace dir
  (se concept tornato), `<project>/.claude/` e `<project>/CLAUDE.md`.
  Produce un albero unificato con metadata di provenienza.
- [ ] **Lineage**: ogni nodo dell'albero risolto ha un riferimento esplicito
  al layer di provenienza (Global / Workspace / Project) e, se
  applicabile, al file sorgente specifico.
- [ ] **Override visibility**: quando un livello override un altro, la
  vista lo segnala (la regola "vincente" mostra anche l'esistenza della
  regola "perdente" in un livello inferiore).
- [ ] **Path configuration**: il path della config globale è il default
  `~/.claude/`, l'utente può override-arlo via config DevDash.

### Non funzionali

- **Performance**: la vista deve essere reattiva su progetti con `.claude/`
  fino a ~50 file (target reale: dimensione attuale di
  `rsrc/project-scaffold/` ~25 file).
- **Resilienza**: file mancanti, permessi negati, encoding non-UTF8 →
  l'app non crasha, mostra placeholder e log.
- **Auto-refresh**: se cambiano file sul filesystem mentre la vista è
  aperta, idealmente la vista si aggiorna (può essere v1 con
  refresh manuale e v2 con file watcher).

## Acceptance Criteria

- [ ] Aprire un progetto vergine → la vista mostra "config = globale
  ereditato", senza richiedere alcun setup.
- [ ] Aprire un progetto con `.claude/` custom → la vista mostra il merge
  con badge di provenienza per ogni nodo.
- [ ] Modificare una regola in `~/.claude/CLAUDE.md` esternamente, refresh
  → la vista riflette la modifica.
- [ ] Una regola override-ata mostra entrambe le versioni (effettiva +
  override-ata) con indicazione di quale livello vince.

## Technical Notes

### Stack

L'implementazione è in C++20 + Dear ImGui (vedi ADR-008). La libreria
markdown `imgui_md` + MD4C (vedi ADR-009) è già disponibile per il
rendering dei file `.md` dentro la vista.

### Modello di dominio (bozza)

- `Project` — path assoluto + metadata (presenza `.claude/`, `.git/`, ecc.).
- `ConfigLayer` — uno fra Global / Workspace / Project. Astrazione che
  punta a un albero filesystem.
- `EffectiveConfig` — risultato del merge dei layer; ogni nodo ha un
  riferimento al layer di provenienza.

### Decisioni architetturali implicate

- **L'app lavora su un singolo project context globale** (variabile
  applicativa). La selezione del project è esterna a questa feature.
- **La vista è sempre disponibile**: non esiste stato "uninitialized"
  che la blocca.

## Open Questions

- **Workspace level**: nel medio periodo il concept "workspace" tornerà
  come orchestrazione di N progetti? Per ora la spec è project-only e
  il `ConfigLayer` *Workspace* è opzionale (può essere assente).
- **Granularità del lineage**: per file `.md` con headings, vogliamo
  mostrare la provenienza per *sezione* (riconoscendo che un livello può
  override una sezione di un file definito in un livello inferiore) o solo
  per *file intero*? MVP: file intero. Granularità più fine come
  evoluzione.
- **Override semantico**: come si "merge" un settings.json global con uno
  project (deep merge JSON), un CLAUDE.md global con uno project
  (concatenazione? sezione-per-sezione?), una lista di hooks
  (sostituzione? unione?). Va deciso layer-per-layer.

## Related

- ADR-008: pivot a C++/Dear ImGui (stack di implementazione).
- ADR-009: scelta libreria markdown.
- ADR-010 (pianificato): architettura del progetto vero.
- `feature-scaffold-management.md`: parte 2/3 della wedge.
- `feature-snapshot-history.md`: parte 3/3 della wedge.
- `backlog/feature-runtime-view-of-truth.md`: estensione futura — confronto
  fra config risolta e cosa Claude ha effettivamente caricato in sessione.
- `archived/feature-claude-context.md`: progenitore di dicembre 2025.
