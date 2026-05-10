---
type: feature
priority: must-have
status: planned
category: core
related: []
supersedes: [feature-claude-context]
created: 2026-05-10
---

# Effective Config View & Scaffold Management

> **Wedge feature di DevDash.** Tutto il resto si misura contro questa.

## Summary

Una vista che risolve, visualizza e mantiene allineata la configurazione
Claude Code di un progetto rispetto a (a) i livelli che la compongono
(globale → workspace → project), (b) gli scaffold dell'utente — set di
configurazione riutilizzabili che l'utente possiede e propaga a piacere
sui propri progetti.

L'utente non deve più "inizializzare" un progetto come azione separata: la
vista è sempre attiva, mostra cosa c'è e cosa manca rispetto allo scaffold
di riferimento, e permette di applicare modifiche in entrambe le direzioni.

## Background

Il framing strategico vive in `.personal/business/analysis/analysis.md`:
nessun tool oggi mostra la *effective configuration* su base global / workspace
/ project nel modo in cui `kubectl config view --merged` o
`eslint --print-config` la mostrano per i loro domini. Tutti gli altri pillar
(workspace management, MCP UI, code graph, memory bank) sono affollati o
commodity. Questa è la casella vuota — la wedge.

Storicamente la stessa idea era stata abbozzata in
`feature-claude-context.md` (dicembre 2025) come "vista a tre livelli
affiancati". L'evoluzione di maggio 2026 cambia la natura della vista: da
*affiancata e statica* a *risolta con lineage* (effective config) +
*allineabile a uno standard utente* (scaffold management). La spec di
dicembre è archiviata; questa la rimpiazza.

## Scope di prodotto

### In scope (MVP)

1. **Effective Config View (statico)** — risoluzione e visualizzazione del
   merge global / workspace / project per il progetto corrente, con
   indicazione della provenienza di ogni regola.
2. **Scaffold Management** — gestione di uno o più scaffold dell'utente
   come cartelle indipendenti, applicazione su un progetto, diff
   bidirezionale (project ↔ scaffold), promote-to-scaffold.
3. **Snapshot & History** — salvataggio di stati intermedi della config
   del progetto corrente (espliciti dell'utente + automatici prima di
   ops distruttive), con restore. Distinto dagli scaffold: snapshot è
   project-scoped, scaffold è cross-project.
4. **Project-as-context** — la dashboard lavora su un singolo progetto
   alla volta. Quale progetto sia "corrente" è impostato fuori da questa
   feature (oggi: argomento o config; futuro: workspace switcher).

### Out of scope MVP, planned per dopo

4. **Runtime View-of-Truth (dinamico)** — confronto della configurazione
   risolta con cosa Claude *ha effettivamente caricato* in una sessione
   reale (parsing dei transcript / log in `~/.claude/projects/<hash>/`).
   Complementare alla vista statica, richiede integrazione separata.

### Out of scope (non in roadmap immediata)

- Workspace orchestration (gestione di N progetti come gruppo).
- Versioning attivo degli scaffold (commit hash, tag, history).
  L'utente versiona la cartella scaffold con git se vuole; DevDash non
  se ne occupa.
- MCP server registry / browser (commodity, integrare via MCP nativo).
- Code graph rendering (vedi `backlog/feature-code-graph.md`; integrazione
  via MCP, non rebuild).

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

### US-2 — Vedere il drift rispetto allo scaffold

**Come** utente con uno scaffold di riferimento
**Voglio** vedere in che cosa la configurazione del progetto corrente
differisce dallo scaffold (o da uno scaffold scelto)
**Per** sapere cosa manca, cosa è stale, cosa è custom mio.

**Acceptance:**

- Per ogni file dello scaffold: mostra se è **assente** nel progetto, **uguale**,
  **modificato** (con diff visibile), o **non presente nello scaffold ma
  presente nel progetto** (custom).
- L'utente può aprire ogni diff side-by-side senza uscire dalla vista.

### US-3 — Applicare lo scaffold a un progetto

**Come** utente
**Voglio** applicare lo scaffold (o una sua selezione) al progetto corrente
**Per** allineare il progetto allo standard senza copia-incolla manuale.

**Acceptance:**

- L'utente sceglie quale scaffold applicare (se più di uno).
- L'utente sceglie quali file/sezioni applicare (default: tutti i file
  diff-erenti che lui non ha marcato come "custom").
- L'azione è transactional: o vanno tutte le scritture, o nessuna.
- Funziona sia su progetti vergini ("primo init") sia su progetti già
  configurati ("re-allineamento"). Niente stato boolean `IsInitialized`.

### US-4 — Promuovere modifiche locali a scaffold

**Come** utente che ha tweakato la configurazione di un progetto e ne è
soddisfatto
**Voglio** promuovere le modifiche allo scaffold
**Per** propagarle ai prossimi progetti che inizializzerò.

**Acceptance:**

- L'utente seleziona quali file modificati promuovere.
- L'utente sceglie se promuovere allo scaffold corrente (sovrascrivendo)
  o salvarli come **scaffold nuovo** (con un nome).
- DevDash non versiona gli scaffold; se l'utente li tiene in git, i
  commit li fa lui con strumenti standard.

### US-5 — Gestire più scaffold

**Come** utente con esigenze diverse (es. scaffold-coding, scaffold-research)
**Voglio** mantenere più scaffold come "snapshot" di configurazione
**Per** scegliere quale applicare quando inizializzo un progetto.

**Acceptance:**

- DevDash mostra la lista degli scaffold trovati in `~/.devdash/scaffolds/`
  (path configurabile).
- L'utente può creare un nuovo scaffold (vuoto, o copiando uno esistente,
  o promuovendo il progetto corrente).
- L'utente può cancellare uno scaffold dalla UI (con conferma).
- Default: il primo scaffold in ordine alfabetico, o uno marcato come
  "default" via metadata.

### US-6 — Vedere config Claude prima di fare nulla

**Come** utente curioso o in fase di valutazione
**Voglio** poter aprire un progetto e vedere la sua config Claude **senza
prima dover applicare uno scaffold**
**Per** non essere costretto a un onboarding prima di poter ispezionare.

**Acceptance:**

- US-1 funziona da subito, anche su progetti vergini.
- US-2/3/4 sono azioni opzionali, mai prerequisito di US-1.

### US-7 — Salvare snapshot della config corrente

**Come** utente che ha tweakato la config del progetto e vuole un punto
di ritorno
**Voglio** salvare uno snapshot nominato dello stato attuale
**Per** poter riprovare modifiche diverse senza paura di perdere il lavoro.

**Acceptance:**

- L'utente clicca "Save snapshot" → inserisce nome + descrizione opzionale.
- Lo snapshot è una copia della cartella `.claude/` (e degli altri pezzi
  scaffold-relevant del progetto, vedi "Modello di dominio") salvata sotto
  `~/.devdash/snapshots/<project-slug>/<timestamp>-<slug>/`.
- Lo snapshot è visibile nella "History" del progetto corrente.
- Lo snapshot è specifico al progetto: non appare nella lista degli
  scaffold riutilizzabili (US-5).

### US-8 — Ripristinare da snapshot

**Come** utente
**Voglio** ripristinare uno snapshot precedente del progetto corrente
**Per** tornare a uno stato salvato.

**Acceptance:**

- L'utente seleziona uno snapshot dalla History → "Restore".
- Il restore è un apply forzato (sovrascrittura): default è ripristinare
  *tutto* lo snapshot, con la possibilità di selezione granulare come
  per US-3.
- Prima del restore, DevDash crea automaticamente un autosnapshot
  (`pre-restore-...`) per garantire che anche il restore sia reversibile.

### US-9 — Autosnapshot prima di operazioni distruttive

**Come** utente
**Voglio** che DevDash crei automaticamente uno snapshot prima di
operazioni che sovrascrivono file (apply scaffold, promote-overwriting,
restore)
**Per** avere sempre un punto di ritorno anche quando non ho ricordato
di farlo manualmente.

**Acceptance:**

- Prima di ogni apply/promote/restore, DevDash crea uno snapshot con
  marker `auto-` e nome che descrive l'azione (`auto-pre-apply-scaffold-coding`,
  `auto-pre-restore-2026-04-30-1500`).
- Gli autosnapshot sono visibili in History, marcati visualmente come
  automatici.
- Politica di pruning: gli ultimi N autosnapshot per project sono
  preservati (default N=10, configurabile); i più vecchi vengono
  cancellati. Gli snapshot espliciti (US-7) **non** vengono mai pruned
  automaticamente.

## Requirements

### Funzionali

- [ ] **Resolver di config**: legge `~/.claude/`, eventuale workspace dir
  (se concept tornato), `<project>/.claude/` e `<project>/CLAUDE.md`.
  Produce un albero unificato con metadata di provenienza.
- [ ] **Discovery scaffold**: scansiona `~/.devdash/scaffolds/` (path
  configurabile) e tratta ogni sottocartella come uno scaffold indipendente.
- [ ] **Diff engine**: confronto file-by-file fra due alberi di filesystem
  (project vs scaffold; project vs effective config). Output strutturato
  consumabile dalla UI.
- [ ] **Apply engine**: scrittura transazionale di un sotto-set selezionato
  di file dallo scaffold al progetto. Backup pre-scrittura raccomandato.
- [ ] **Promote engine**: copia di un sotto-set di file dal progetto allo
  scaffold (existing) o a una nuova cartella scaffold (new).
- [ ] **Snapshot engine**: salvataggio della config corrente del project
  in `~/.devdash/snapshots/<project-slug>/<timestamp>-<slug>/`. Trigger
  espliciti (US-7) e automatici prima di apply/promote/restore (US-9).
  Pruning automatico solo per autosnapshot (last N preservati).
- [ ] **Restore engine**: apply forzato di uno snapshot al project di
  origine (US-8). Tecnicamente è una variante dell'apply engine con
  default conflict-resolution = overwrite invece di skip-with-confirmation.
- [ ] **Path configuration**: `~/.devdash/scaffolds/` e
  `~/.devdash/snapshots/` sono i default, l'utente può override-arli via
  config.

### Non funzionali

- **Performance**: la vista deve essere reattiva su progetti con `.claude/`
  fino a ~50 file e scaffold fino a ~50 file (target reale: la dimensione
  attuale di `rsrc/project-scaffold/` è ~25 file).
- **Resilienza**: file mancanti, permessi negati, encoding non-UTF8 →
  l'app non crasha, mostra placeholder e log.
- **Trasparenza**: l'utente vede *cosa farebbe* l'app prima che lo faccia.
  Niente "magic side-effects".
- **Idempotenza**: applicare lo stesso scaffold due volte allo stesso
  progetto produce lo stesso risultato la seconda volta della prima
  (= no-op se non ci sono drift).

## Acceptance Criteria (verificabili)

- [ ] Aprire un progetto vergine → US-1 mostra "config = globale ereditato".
- [ ] Aprire un progetto già scaffold-ato → US-2 mostra zero drift se non ci
  sono modifiche locali, drift puntuale se ci sono.
- [ ] Applicare uno scaffold a un progetto vergine → l'output è
  bit-identical a una copia diretta di `~/.devdash/scaffolds/<name>/` nel
  progetto (modulo file marcati `.gitkeep` o equivalenti).
- [ ] Modificare un file nel progetto, poi promuoverlo allo scaffold →
  ri-aprire un altro progetto vergine e applicare lo scaffold → la modifica
  è presente.
- [ ] Cancellare il path `~/.devdash/scaffolds/` → la lista scaffold è
  vuota, US-3/4/5 mostrano stato "no scaffold available", US-1 funziona
  comunque.
- [ ] Salvare snapshot manuale, fare modifiche, salvare un secondo snapshot,
  restore al primo → il progetto torna allo stato del primo snapshot e un
  autosnapshot `pre-restore-...` appare nella history.
- [ ] Applicare uno scaffold a un progetto già configurato → un autosnapshot
  `pre-apply-...` appare nella history e contiene la config pre-apply
  intatta.
- [ ] Lasciare accumulare > N autosnapshot (default N=10) → i più vecchi
  vengono pruned, gli snapshot espliciti restano.

## Technical Notes

### Stack

L'implementazione è in C++20 + Dear ImGui sul nuovo PoC (vedi ADR-008).
La libreria markdown `imgui_md` + MD4C (vedi ADR-009) è già disponibile
per il rendering dei file `.md` dentro la vista.

### Modello di dominio (bozza, da raffinare in fase architetturale)

- `Project` — path assoluto + metadata derivati (presenza `.claude/`,
  `.git/`, ecc.).
- `Scaffold` — path assoluto a `~/.devdash/scaffolds/<name>/` + metadata.
- `Snapshot` — path assoluto a `~/.devdash/snapshots/<project-slug>/<timestamp>-<slug>/`
  + metadata (tipo: explicit/auto, descrizione, timestamp, riferimento al
  project di origine). Strutturalmente identico a uno scaffold; differisce
  solo per intent (project-scoped) e per UI (esposto come "History" del
  project, non come "library").
- `ConfigLayer` — uno fra Global / Workspace / Project. Astrazione che
  punta a un albero filesystem.
- `EffectiveConfig` — risultato del merge dei layer; ogni nodo ha un
  riferimento al layer di provenienza.
- `DiffEntry` — { path, kind ∈ {missing, unchanged, modified, custom},
  diff_payload? }.

### Decisioni architetturali implicate (da fissare nell'ADR-010)

- **L'app lavora su un singolo project context globale** (variabile
  applicativa). La selezione del project è esterna.
- **Lo scaffold è un artefatto filesystem dell'utente**, non un blob
  embedded nel binario.
- **La vista è sempre disponibile**: non esiste stato "uninitialized".
- **Niente Vault@Obsidian** come componente architetturale.

### Anti-pattern da evitare (da error precedente)

- Form a campi liberi compilati a freddo prima di vedere cosa succede.
- Stato boolean `IsConfigured` / `IsInitialized` che blocca riapplicazione.
- Scaffold come risorse embedded nel binario (impossibile da aggiornare
  senza ricompilare).
- Template con string-replacement opachi (`{PROJECT_NAME}`,
  `{LANGUAGE_SPECIFIC_STANDARDS}`) — preferire file copy + manual edit
  visibile all'utente.

## Open Questions

- **Workspace level**: nel medio periodo il concept "workspace" tornerà
  come orchestrazione di N progetti? Per ora la spec è project-only e
  il `ConfigLayer` *Workspace* è opzionale (può essere assente). Da
  decidere quando si torna sulla questione.
- **Granularità del diff**: file intero, sezione di markdown
  (per `CLAUDE.md` con headings), riga? Suggerimento: file-intero per
  l'MVP, granularità più fine come evoluzione.
- **"Scaffold default"**: meccanismo di marker? File `.devdash-default`
  nella cartella scaffold? Setting in config DevDash? Da decidere quando
  si arriva alla UX dello selector.
- **Conflict resolution su apply**: cosa succede se l'utente applica uno
  scaffold ma il file di destinazione è stato modificato e marcato come
  "custom"? Default: skip (chiede conferma); alternative possibili
  (force, three-way merge). MVP: skip.
- **Scaffold seed**: il primo scaffold dell'utente da dove arriva?
  - Opzione A: l'utente lo crea da zero o lo importa da un'altra source.
  - Opzione B: DevDash propone di seedare il primo scaffold da
    `rsrc/project-scaffold/` (l'attuale embedded) come point-of-departure.
  - L'opzione B è gentile per l'onboarding ma reintroduce risorse embedded
    nel binario in forma minore. Da discutere.
- **Variabili / placeholder**: la versione attuale dello scaffold ha
  `{PROJECT_NAME}`, `{TECH_STACK_DESCRIPTION}` ecc. che si risolvono
  all'apply. Manteniamo questo meccanismo (con UI esplicita per i valori)
  o lo eliminiamo del tutto (l'utente customizza il file dopo l'apply)?
  Inclinazione: **eliminarlo** per coerenza con "trasparenza, no magic".

## Related

- ADR-008: pivot a C++/Dear ImGui (stack di implementazione).
- ADR-009: scelta libreria markdown (per rendering dei file markdown
  dentro la vista).
- ADR-010 (pianificato): architettura del progetto vero — questa spec
  ne è la bussola.
- `archived/feature-claude-context.md`: progenitore di dicembre 2025,
  superseded da questa spec.
- `.personal/business/analysis/analysis.md`: framing strategico del
  perché questa è la wedge feature.
