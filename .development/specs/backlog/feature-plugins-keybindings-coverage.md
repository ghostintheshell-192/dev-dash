---
type: feature
priority: nice-to-have
status: backlog
category: core
part_of: effective-config
related: [feature-effective-config-view, feature-global-settings-coverage]
depends_on: [feature-global-settings-coverage]
created: 2026-06-10
---

# Plugins & Keybindings Coverage

La effective config view copre CLAUDE.md, rules, memory, skills, agents,
MCP e hooks — ma non ciò che arriva dai **plugin** di Claude Code
(`~/.claude/plugins/`: skills e command che l'utente non vede come file
propri) né i **keybindings** (`~/.claude/keybindings.json`).

## Scope

- **Plugins**: enumerare i plugin installati e le capability che portano
  (skills/commands), con badge che distingua "tuo" da "portato da plugin".
  Risponde alla domanda "perché Claude ha questa skill che io non ho mai
  scritto?".
- **Keybindings**: sezione dedicata via il renderer JSON generico di
  `feature-global-settings-coverage` (probabilmente non serve un adapter
  semantico: il tree generico basta).

## Note

- Il formato della directory plugins è infrastruttura Anthropic non
  documentata stabilmente: usare l'approccio generico/resiliente della spec
  collegata, non hardcodare lo schema.
- Dipende da `feature-global-settings-coverage` per il renderer generico.

## Acceptance (di massima)

1. Una skill proveniente da un plugin compare nella sezione Skills con
   provenienza esplicita.
2. `keybindings.json` (se esiste) è visibile nella config view.
