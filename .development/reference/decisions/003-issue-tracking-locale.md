# ADR-003: Issue tracking locale vs GitHub Issues

**Data**: 2024-12-02
**Status**: Accepted
**Impact**: medium
**Sommario**: Adotta un issue tracking locale in file markdown sotto `.personal/issues/` per le issue personali, riservando GitHub Issues alle issue pubbliche e ai bug report esterni.

## Contesto

Per lo sviluppo in solitaria servono due tipi di issue:

1. **Issue pubbliche**: bug report da utenti, feature request esterne
2. **Issue personali**: task interni, idee, tech debt, note di lavoro

GitHub Issues ha overhead per issue personali:

- Richiede browser, login, network
- Visibilità pubblica non sempre desiderata
- Interfaccia pesante per note veloci

## Decisione

Issue tracking locale in `.personal/issues/` per issue personali. GitHub Issues per issue pubbliche/bug report.

## Rationale

- Markdown file = editabile ovunque, versionato con git (se si vuole)
- Zero overhead: creare issue = creare file
- Integrato con DevDash: navigazione e filtri nativi
- Offline-first

## Struttura issue

```yaml
# .personal/issues/001-fix-column-resize.md
---
id: 001
title: Fix column resize lag
status: open  # open, in-progress, done
priority: medium
created: 2024-12-01
tags: [ui, performance]
---

## Descrizione
...
```

## Conseguenze

- **Pro**: Velocità, offline, integrato con workflow esistente
- **Contro**: Nessuna collaboration (accettabile per sviluppo solitario)
- **Mitigazione**: Per progetti collaborativi, usare GitHub Issues
