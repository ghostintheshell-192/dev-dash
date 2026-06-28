---
captured: 2026-06-14
status: promoted-to-adr
promoted_to: [../../.development/reference/decisions/015-scaffold-templating.md, ../../.development/reference/decisions/016-git-managed-scaffold-versioning.md, ../../.development/specs/planned/feature-scaffold-templating.md]
promoted_at: 2026-06-28
context: "sessione modello-ADR (branch refactor/adr-model); task 7 rimandato a sessione dedicata"
tags: [scaffold, adr-model, promote-engine, dogfooding, discoverability]
---

# Allineare lo scaffold al nuovo modello ADR

## Cos'è

Il modello ADR è stato rimesso in sesto nel repo dev-dash (scala `Impact`,
`_TEMPLATE.md`, `Sommario` estratto, `key-decisions.md` caricato via `@include`,
generatori aggiornati). Lo **scaffold `rsrc/project-scaffold/` non è stato
allineato** — rimandato per non allargare la sessione.

Da portare nello scaffold:

- `generate-claude-config.sh` aggiornato (generico, copia diretta OK).
- `_TEMPLATE.md` + `README.md` ADR (generici).
- Sezione "Key Decisions" + `@include key-decisions.md` nel `CLAUDE.md` dello
  scaffold — **preservando i placeholder** `{PROJECT_NAME}` ecc.
- `key-decisions.md` placeholder (generabile eseguendo lo script dentro lo
  scaffold: 0 ADR → file con solo header/footer).
- **Variante template di `generate-architecture.sh`**: lo script attuale è
  dev-dash-specifico (sezione "Project Configuration" con `PROJECT_NAME`,
  `SOURCE_DIRS=app/src`, e un `generate_project_header()` con il Layer Overview
  di dev-dash hardcoded). Va creata una versione con quella sezione a
  placeholder. Stesso discorso per `extract-summary.sh` (oggi copre solo
  C#/C++).

## Perché merita attenzione

1. Senza allineamento, ogni progetto nuovo nasce **senza** struttura ADR né il
   meccanismo di discoverability/loading — la lacuna che questa sessione ha
   appena chiuso su dev-dash.
2. **Lacuna del PromoteEngine** (emersa ragionando sul metodo): il promote
   dall'app — il flusso dogfooding-corretto secondo ADR-013 — oggi
   (presumibilmente) non copre `.development/scripts/` né il modello-ADR, e un
   promote-copia del `CLAUDE.md` clobbererebbe i placeholder dello scaffold.
   Idealmente "evolvi il meccanismo nel repo → propaga allo scaffold" dovrebbe
   essere un flusso supportato. Verificare cosa il PromoteEngine copre davvero.

## Next-step minimo

Sessione dedicata: (a) verificare la copertura reale del PromoteEngine;
(b) decidere promote vs copia manuale per le parti generiche; (c) creare le
varianti template degli script dev-dash-specifici. Vedi task #7 e il tech-debt
`.development/tech-debt/scaffold-architecture-scripts.md` (aggiornato e
de-.NET-izzato il 2026-06-14), che possiede la parte "architecture scripts".

## Chiusura 2026-06-28

Entrambe le parti dell'idea sono state affrontate:

- **Allineamento modello ADR allo scaffold** → *fatto* e committato (branch
  `chore/scaffold-adr-alignment`, mergiato in `develop`): generatori, template
  ADR, `key-decisions.md` + `@include`, varianti template degli script. Risolto
  anche un bug latente (docs-update chiamava un generatore assente nello scaffold).
- **Lacuna PromoteEngine** → *chiarita e promossa*. Verificato che PromoteEngine è
  copia pura (nessuna sostituzione), e che la sostituzione forward non esisteva
  *by design* (era anti-pattern in `feature-scaffold-management`). Ri-decisa con
  Valentina: sostituzione esplicita guidata da manifest → **ADR-015**; il
  versioning git degli scaffold (che rende il promote tracciabile) → **ADR-016**;
  il lavoro → `feature-scaffold-templating` (planned).

Ground truth da qui in avanti: gli artefatti in `promoted_to`.
