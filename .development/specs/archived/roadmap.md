# Roadmap DevDash

## Fase 0: Prototipo UI ✅

**Status**: Completato

- [x] Layout base con sidebar collassabile
- [x] Workspace switcher (Coding/Writing)
- [x] Lista progetti con indicatori
- [x] File tree navigabile
- [x] Tab system (personal, docs, issues, ADR, config)
- [x] Panel Claude Code mockato
- [x] Settings modal
- [x] Tema scuro + accent ambra

**Output**: `devdash-prototype.tsx`

---

## Fase 1: Filesystem reale

**Obiettivo**: Collegare UI al filesystem locale.

### 1.1 Backend minimo

- [ ] Setup progetto Electron (o Tauri per bundle più leggero)
- [ ] API per leggere directory
- [ ] API per leggere contenuto file markdown
- [ ] API per listare workspace da config

### 1.2 Integrazione frontend

- [ ] Sostituire mock data con chiamate API
- [ ] Refresh automatico su file change (watch)
- [ ] Gestione errori (path non esiste, permessi)

### 1.3 Parsing configurazioni

- [ ] Leggere `~/.claude/CLAUDE.md` per lista workspace
- [ ] Parsare `goto.yaml` embedded
- [ ] Rilevare progetti con `.personal/` e `docs/`

**Milestone**: Navigare progetti reali e vedere file markdown.

---

## Fase 2: Editing base

**Obiettivo**: Modificare file senza uscire dalla dashboard.

### 2.1 Editor markdown

- [ ] Monaco Editor o CodeMirror integrato
- [ ] Syntax highlighting markdown
- [ ] Save con Ctrl+S
- [ ] Unsaved changes indicator

### 2.2 Preview

- [ ] Render markdown → HTML
- [ ] Split view edit/preview
- [ ] Supporto frontmatter YAML

**Milestone**: Editare `.personal/CURRENT-STATUS.md` direttamente.

---

## Fase 3: Issue tracking

**Obiettivo**: Gestire issue personali integrate.

### 3.1 Struttura issue

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

### 3.2 Features

- [ ] Lista issue con filtri (status, priority)
- [ ] Quick create issue
- [ ] Cambiare status inline
- [ ] Link issue → spec

**Milestone**: Creare e gestire issue senza lasciare DevDash.

---

## Fase 4: Config management

**Obiettivo**: Visualizzare e modificare configurazioni Claude Code.

### 4.1 Effective configuration

- [ ] Leggere config dai tre livelli
- [ ] Calcolare merge con precedenza
- [ ] Mostrare diff tra livelli
- [ ] Indicare origine di ogni setting

### 4.2 Editing config

- [ ] Edit config con syntax highlighting YAML/MD
- [ ] Validazione sintassi
- [ ] Warning per override nascosti

**Milestone**: Capire "perché Claude si comporta così" in un click.

---

## Fase 5: Integrazione Claude Code

**Obiettivo**: Lanciare e comunicare con Claude Code.

### 5.1 Launch con contesto

- [ ] Bottone "Open in Claude Code" su progetto
- [ ] Pre-caricare `.personal/INDEX.md`
- [ ] Passare working directory corretta

### 5.2 MCP integration (opzionale)

- [ ] Configurare Vault@Claude come MCP endpoint
- [ ] Claude Code può leggere/scrivere via MCP
- [ ] DevDash come orchestratore

**Milestone**: Click su issue → Claude Code aperto con contesto.

---

## Fase 6: Polish

- [ ] Keyboard shortcuts (Ctrl+P per progetti, etc.)
- [ ] Command palette
- [ ] Temi (oltre dark)
- [ ] Sync settings tra macchine
- [ ] Onboarding per nuovi utenti

---

## Non in scope (per ora)

- Git integration profonda (usa git CLI o VS Code)
- Code editing vero (usa VS Code)
- Collaboration (è per sviluppo solitario)
- Cloud sync (usa git per quello)

---

## Decision log

| Data | Decisione | Rationale |
|------|-----------|-----------|
| 2024-12-02 | Electron vs Tauri | TBD - Tauri più leggero ma meno maturo |
| 2024-12-02 | Monaco vs CodeMirror | TBD - Monaco più features, CodeMirror più leggero |
