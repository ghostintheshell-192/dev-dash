---
type: feature
priority: must-have
status: in-progress
category: core
part_of: wedge
related: [feature-effective-config-view, feature-snapshot-history]
depends_on: [feature-effective-config-view][../../reference/technical/resource-model.md]
created: 2026-05-10
---

# Scaffold Management

> **Parte 2/3 della wedge feature di DevDash.** Permette all'utente di
> mantenere uno o più scaffold come artefatti propri (cartelle), applicarli
> ai progetti, vedere il drift fra progetto e scaffold, e propagare modifiche
> in entrambe le direzioni. Le altre due parti sono
> `feature-effective-config-view.md` (parte 1, mostra cosa Claude vede) e
> `feature-snapshot-history.md` (parte 3, snapshot e restore).

## Summary

Lo scaffold è un **artefatto utente**, non un blob embedded nel binario:
una cartella sotto `~/.devdash/scaffolds/<name>/` che contiene un set di
file (CLAUDE.md, .claude/, .githooks/, ecc.) che l'utente vuole replicare
nei propri progetti.

DevDash:
- mostra la lista degli scaffold disponibili,
- permette di applicare uno scaffold a un progetto (con o senza
  selezione granulare dei file),
- mostra il diff bidirezionale fra progetto corrente e scaffold scelto,
- permette di promuovere modifiche locali del progetto allo scaffold
  (sovrascrittura) o salvarle come scaffold nuovo.

L'utente non "inizializza" un progetto come azione separata e una-tantum:
applicare lo scaffold è un'azione ripetibile, idempotente, sempre
disponibile.

## Background

Storicamente, il PoC `.NET/Avalonia` aveva un `ScaffoldService` che
trattava lo scaffold come *risorsa embedded* nel binario, applicabile
una sola volta tramite un form a 3 campi liberi (Description, TechStack,
Language) prima di vedere cosa sarebbe successo. Questo approccio è
stato giudicato macchinoso dall'utente:

- Decisione cieca (form prima di preview).
- All-or-nothing (no granular apply).
- One-shot binario (`IsConfigured` boolean lock-in).
- Template stale by design (embedded → ricompilare per aggiornarlo).
- Disconnesso dal resto della UX.

Questa spec rifonda il concetto: lo scaffold è dell'utente, vive sul
filesystem, ed è un punto di riferimento *continuo* contro cui il
progetto si misura.

## User Stories

### US-1 — Vedere il drift rispetto allo scaffold

**Come** utente con uno scaffold di riferimento
**Voglio** vedere in che cosa la configurazione del progetto corrente
differisce dallo scaffold (o da uno scaffold scelto)
**Per** sapere cosa manca, cosa è stale, cosa è custom mio.

**Acceptance:**

- Per ogni file dello scaffold: mostra se è **assente** nel progetto, **uguale**,
  **modificato** (con diff visibile), o **non presente nello scaffold ma
  presente nel progetto** (custom).
- L'utente può aprire ogni diff side-by-side senza uscire dalla vista.

### US-2 — Applicare lo scaffold a un progetto

**Come** utente
**Voglio** applicare lo scaffold (o una sua selezione) al progetto corrente
**Per** allineare il progetto allo standard senza copia-incolla manuale.

**Acceptance:**

- L'utente sceglie quale scaffold applicare (se più di uno disponibile).
- L'utente sceglie quali file/sezioni applicare (default: tutti i file
  diff-erenti che non sono marcati come "custom").
- L'azione è transazionale: o vanno tutte le scritture, o nessuna.
- Funziona sia su progetti vergini ("primo init") sia su progetti già
  configurati ("re-allineamento"). Niente stato boolean `IsInitialized`.
- Prima dell'apply, DevDash crea automaticamente un autosnapshot
  (vedi `feature-snapshot-history.md`).

### US-3 — Promuovere modifiche locali a scaffold

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
- In caso di sovrascrittura, l'utente vede il diff prima di confermare.

### US-4 — Gestire più scaffold

**Come** utente con esigenze diverse (es. scaffold-coding, scaffold-research)
**Voglio** mantenere più scaffold come "snapshot di configurazione
riutilizzabili"
**Per** scegliere quale applicare quando inizializzo un progetto.

**Acceptance:**

- DevDash mostra la lista degli scaffold trovati in `~/.devdash/scaffolds/`
  (path configurabile).
- L'utente può creare un nuovo scaffold (vuoto, copiando uno esistente,
  o promuovendo il progetto corrente — vedi US-3).
- L'utente può cancellare uno scaffold dalla UI (con conferma).
- Default: il primo scaffold in ordine alfabetico, o uno marcato come
  "default" via metadata.

## Requirements

### Funzionali

- [x] **Discovery scaffold**: scansiona `~/.devdash/scaffolds/` (path
  configurabile) e tratta ogni sottocartella come uno scaffold indipendente.
- [x] **Diff engine**: confronto file-by-file fra due alberi di filesystem
  (project ↔ scaffold). Output strutturato consumabile dalla UI.
- [x] **Apply engine**: scrittura transazionale di un sotto-set selezionato
  di file dallo scaffold al progetto. Backup pre-scrittura (autosnapshot)
  obbligatorio per ogni apply.
- [x] **Promote engine**: copia di un sotto-set di file dal progetto allo
  scaffold (existing) o a una nuova cartella scaffold (new).
- [x] **Path configuration**: `~/.devdash/scaffolds/` è il default,
  l'utente può override-arlo via config.

### Non funzionali

- **Trasparenza**: l'utente vede *cosa farebbe* l'app prima che lo faccia.
  Niente "magic side-effects".
- **Idempotenza**: applicare lo stesso scaffold due volte allo stesso
  progetto produce lo stesso risultato la seconda volta della prima
  (= no-op se non ci sono drift).
- **Resilienza**: file mancanti, permessi negati, encoding non-UTF8 →
  l'app non crasha, mostra placeholder e log.

## Acceptance Criteria

- [x] Aprire un progetto già scaffold-ato → mostra zero drift se non ci
  sono modifiche locali, drift puntuale se ci sono.
- [x] Applicare uno scaffold a un progetto vergine → l'output è
  bit-identical a una copia diretta di `~/.devdash/scaffolds/<name>/` nel
  progetto (modulo file marcati `.gitkeep` o equivalenti).
- [x] Modificare un file nel progetto, poi promuoverlo allo scaffold →
  ri-aprire un altro progetto vergine e applicare lo scaffold → la modifica
  è presente.
- [x] Cancellare il path `~/.devdash/scaffolds/` → la lista scaffold è
  vuota, le US 1-4 mostrano stato "no scaffold available". La feature
  parte 1 (effective config view) funziona comunque.
- [x] Apply a un progetto già configurato → un autosnapshot `pre-apply-...`
  appare nella history (vedi parte 3) e contiene la config pre-apply intatta.

## Technical Notes

### Modello di dominio

- `Scaffold` — path assoluto a `~/.devdash/scaffolds/<name>/` + metadata
  (nome, descrizione opzionale, marker default opzionale).
- `DiffEntry` — { path, kind ∈ {missing, unchanged, modified, custom},
  diff_payload? }.

### Decisioni architetturali implicate

- **Lo scaffold è un artefatto filesystem dell'utente**, non un blob
  embedded nel binario.
- **Niente stato persistente "isInitialized"** sul progetto.
- **Apply è transazionale**: usa staging directory + atomic move o
  equivalente per garantire che un fallimento a metà non lasci il progetto
  in stato inconsistente.

### Anti-pattern da evitare

- Form a campi liberi compilati a freddo prima di vedere cosa succede.
- Stato boolean `IsConfigured` / `IsInitialized` che blocca riapplicazione.
- Scaffold come risorse embedded nel binario.
- Template con string-replacement opachi (`{PROJECT_NAME}`,
  `{LANGUAGE_SPECIFIC_STANDARDS}`) — preferire file copy + manual edit
  visibile all'utente.

## Open Questions

- **Granularità del diff**: file intero (semplice) o sezione di markdown
  (per `CLAUDE.md` con headings)? MVP: file intero. Granularità più fine
  come evoluzione.
- **"Scaffold default"**: meccanismo di marker? File `.devdash-default`
  nella cartella scaffold? Setting in config DevDash?
- **Conflict resolution su apply**: cosa succede se l'utente applica uno
  scaffold ma il file di destinazione è stato modificato e marcato come
  "custom"? Default: skip-with-confirmation; alternative possibili
  (force, three-way merge). MVP: skip.
- **Scaffold seed**: il primo scaffold dell'utente da dove arriva?
  - Opzione A: l'utente lo crea da zero o lo importa da un'altra source.
  - Opzione B: DevDash propone di seedare il primo scaffold da
    `rsrc/project-scaffold/` (l'attuale embedded) come point-of-departure.
  - L'opzione B è gentile per l'onboarding ma reintroduce risorse
    embedded nel binario in forma minore.
- **Variabili / placeholder**: la versione attuale dello scaffold ha
  `{PROJECT_NAME}`, `{TECH_STACK_DESCRIPTION}` ecc. che si risolvevano
  all'apply. Manteniamo questo meccanismo (con UI esplicita per i valori)
  o lo eliminiamo del tutto (l'utente customizza il file dopo l'apply)?
  Inclinazione: **eliminarlo** per coerenza con "trasparenza, no magic".

- **Permessi `Bash(...)` per stack** (emerso 2026-05-10 nel cleanup post-pivot):
  i progetti reali hanno bisogno di permessi diversi a seconda dello stack
  (es. `Bash(cmake:*)` per C++, `Bash(dotnet build:*)` per .NET,
  `Bash(cargo:*)` per Rust). Lo scaffold corrente in `rsrc/project-scaffold/`
  contiene solo permessi language-agnostic (`find`, `grep`, `gh*`, `WebSearch`).
  Tre opzioni:
  - **A**: scaffold per-stack — `~/.devdash/scaffolds/coding-cpp/`,
    `coding-csharp/`, ecc. — ognuno con i propri permessi di default.
    Più scaffold, ma ogni stack è esplicito.
  - **B**: scaffold "intelligente" che rileva lo stack del progetto target
    all'apply (presenza di `CMakeLists.txt`, `*.csproj`, `Cargo.toml`) e
    aggiunge i permessi appropriati. Più magia, meno scaffold da gestire.
  - **C**: lo scaffold contiene un superset di permessi commentati;
    l'utente decommenta quelli rilevanti dopo l'apply. Trasparente ma
    manuale.
  Inclinazione iniziale: **A**, coerente con la filosofia "scaffold come
  artefatto utente esplicito, no magic".

## Related

- `feature-effective-config-view.md`: parte 1/3 (prerequisito concettuale).
- `feature-snapshot-history.md`: parte 3/3 (autosnapshot prima di apply
  vive in quella spec).
- ADR-008, ADR-009: stack di implementazione.
