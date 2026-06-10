---
type: feature
priority: must-have
status: planned
category: infrastructure
part_of: release
related: [feature-release-readiness]
depends_on: []
created: 2026-06-10
---

# Agnostic Automation (two-level)

Migra dev-dash al modello di automazione a due livelli con entry point
standard. Implementa ADR-012 (fase 1 del piano di migrazione). Chiude il
tech-debt `project-githooks-not-active`.

## Goal

Hook e CI di dev-dash girano dal progetto stesso, non dal workspace; gli
orchestratori sono agnostici rispetto allo stack e la conoscenza C++/CMake
vive solo negli entry point. Il workspace resta congelato finché tutti i
progetti non sono migrati.

## Work Breakdown

### Fase 1 — Entry point standard

- [ ] `.development/automation/build.sh` (cmake --build, preset parametrico)
- [ ] `.development/automation/test.sh` (ctest; "no tests yet" exit 0 finché
      la fase 1 di release-readiness non è fatta)
- [ ] `.development/automation/format-check.sh` (clang-format; no-op
      dichiarato finché manca `.clang-format`)
- [ ] `.development/automation/docs-update.sh` (aggrega generate-architecture,
      generate-index, update-tech-debt-index)

### Fase 2 — Hook orchestratori

- [ ] Ridurre `.githooks/pre-commit.d/02-clang-format` a chiamata di
      `format-check.sh`
- [ ] `04-generate-architecture` + nuovo modulo index → chiamano
      `docs-update.sh` (porta nel progetto l'equivalente del `07-generate-index`
      workspace, oggi assente)
- [ ] Decidere il destino di `05-generate-readme-status` (workspace): lo
      script che invoca non è mai esistito — NON portarlo, registrare la
      decisione
- [ ] Verificare che 00-branch-protection, 01-security, 05-spec-workflow
      restino agnostici (lo sono già)

### Fase 3 — Attivazione

- [ ] `.development/automation/bootstrap.sh`: setta `core.hooksPath .githooks`
      locale, verifica prerequisiti (python3, clang-format se richiesto),
      stampa cosa ha attivato
- [ ] Eseguire bootstrap su questo clone → branch protection torna effettiva
- [ ] Aggiornare `.claude/rules/workflow.md` (sezione hooks + bootstrap)
- [ ] Chiudere tech-debt `project-githooks-not-active`

### Fase 4 — Scaffold di riferimento

- [ ] Aggiornare lo scaffold DevDash in `~/.devdash/scaffolds/` con la nuova
      struttura (`.githooks/` orchestratori + `automation/` placeholder), così
      i progetti futuri la ricevono via apply

## Acceptance

1. `git commit` su questo clone esegue gli hook di progetto (verificabile:
   tentato commit su develop → bloccato da 00-branch-protection).
2. Gli hook non contengono comandi specifici dello stack (grep "cmake\|ctest\|
   clang-format" in `.githooks/` → solo negli entry point).
3. La CI di release-readiness consuma `build.sh`/`test.sh` senza conoscere
   CMake.
4. Il workspace `/data/repos/.git-hooks/` non è toccato (decommissionamento
   solo a migrazione completa di tutti i progetti, ADR-012 fase 3).

## Out of Scope

- Migrazione di sheet-atlas (fase 2 del piano ADR-012, decisione separata)
- Rimozione di `core.hooksPath` globale e del workspace (fase 3, ultima)
- Manifest dichiarativo (alternativa scartata in ADR-012)
