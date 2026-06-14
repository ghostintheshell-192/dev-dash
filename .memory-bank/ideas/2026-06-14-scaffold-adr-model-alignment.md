---
captured: 2026-06-14
status: parked
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
varianti template degli script dev-dash-specifici. Vedi task #7.
