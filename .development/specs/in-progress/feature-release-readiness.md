---
type: feature
priority: must-have
status: in-progress
category: infrastructure
part_of: release
related: [feature-snapshot-history, feature-scaffold-management]
depends_on: []
created: 2026-06-10
---

# Release Readiness

Da skeleton funzionante a programma installabile. Implementa ADR-011.

## Goal

Su una macchina Linux pulita (con driver Vulkan), installare DevDash con due
comandi ed eseguirlo subito, senza build tree, senza path hardcoded, senza
fallimenti silenziosi al primo avvio.

## User Stories

- **US-1**: Come utente, scarico il tarball della release (o lancio
  `cmake --install`), eseguo `dev-dash` e l'app parte con i font giusti.
- **US-2**: Come utente, al primo avvio l'app prepara da sola `~/.devdash/`
  e ogni errore di I/O è visibile, non silenziato.
- **US-3**: Come sviluppatrice, ogni push su develop/main è buildato e
  testato dalla CI; un tag `v*` produce da solo la GitHub Release col
  tarball.

## Work Breakdown

### Fase 1 — Test target (sblocca tutto il resto) ✅

- [x] Catch2 v3 via CPM, target `dev-dash-tests`, `enable_testing()` + ctest
- [x] Test `SnapshotService` su tmpdir: save explicit/auto, list, restore,
      prune
- [x] Test `ApplyEngine`: apply/skip/fail counts, force overwrite
- [x] Entry point `.development/automation/test.sh` (ADR-012)

> Nota: `services/` estratto in libreria statica `dev-dash-services` per
> linkare la logica senza lo stack grafico. `-Wall -Wextra` abilitati sui
> nostri target (scoped via interface lib), `-Werror` rimandato alla CI.

### Fase 2 — Installabilità ✅

- [x] `project(... VERSION 0.1.0)` + `version.h` generato via `configure_file`
- [x] Risoluzione asset a due tentativi (adiacente all'eseguibile, poi
      `../share/dev-dash/assets/`)
- [x] DejaVu Sans bundled in `app/assets/fonts/` (rimuove il path di sistema
      hardcoded in `font_library.cpp`)
- [x] Init `~/.devdash/{scaffolds,snapshots}` in `App::Init()`, errori in UI
- [x] `install()` rules con `GNUInstallDirs` (bin, asset, desktop entry)
- [x] `dev-dash.desktop` in `app/assets/`
- [x] CPack TGZ: `dev-dash-X.Y.Z-linux-x86_64.tar.gz`
- [x] Smoke test manuale: install in prefix pulito + run

### Fase 3 — CI e release

- [ ] `.github/workflows/ci.yml`: push/PR su develop/main, ubuntu-latest,
      chiama gli entry point `build.sh` + `test.sh`
- [ ] `.github/workflows/release.yml`: su tag `v*`, build Release + cpack +
      GitHub Release
- [ ] Eliminare `ci.yml.disabled`
- [ ] `CHANGELOG.md` iniziale (Keep-a-Changelog)

### Fase 4 — Documentazione utente ✅

- [x] Rewrite `docs/SETUP.md`: prerequisiti (GCC, CMake ≥3.28, Ninja, Vulkan),
      build, install, primo avvio
- [x] Rewrite `docs/architecture.md`: layered split reale (oggi descrive
      Avalonia)
- [x] api-design.md: aggiungere `PromoteEngine` e gli adapter section-aware
      (oggi implementati ma non documentati)

## Acceptance

1. `ctest` verde in locale e in CI.
2. `cmake --install build --prefix /tmp/devdash-test && /tmp/devdash-test/bin/dev-dash`
   parte con font corretti e crea `~/.devdash/` se assente.
3. Tag `v0.1.0` su main produce una GitHub Release con tarball funzionante.
4. `docs/SETUP.md` seguito alla lettera su macchina pulita porta a un'app
   che gira.

## Out of Scope

- AppImage / .deb / multi-OS (estensioni future, ADR-011)
- Syntax highlighting, UX rework dei pannelli (tech-debt esistenti)
- Generazione automatica del changelog (git-cliff) — manuale per ora
