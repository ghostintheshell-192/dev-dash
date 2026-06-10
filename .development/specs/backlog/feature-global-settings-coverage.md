---
type: feature
priority: nice-to-have
status: backlog
category: core
part_of: effective-config
related: [feature-effective-config-view, feature-plugins-keybindings-coverage]
depends_on: []
created: 2026-06-10
---

# Global Settings Coverage (parsing generalizzato dei JSON di .claude)

Oggi `SettingsParser` estrae da `settings.json` solo MCP server e hook
entries. Permissions, env, model, statusline e ogni chiave futura sono
invisibili nella effective config view. Obiettivo: copertura completa e
resiliente ai cambiamenti che Anthropic farà all'infrastruttura dei file.

## Approccio: ibrido semantico + generico

Due livelli di parsing, in cascata:

1. **Adapter semantici** (esistenti e nuovi): le chiavi che DevDash
   *capisce* (mcpServers, hooks, permissions, env) hanno rendering dedicato
   con badge layer, merge cross-layer, descrizioni.
2. **Renderer JSON generico** (nuovo): tutto ciò che non ha un adapter
   semantico viene mostrato come albero chiave/valore (nlohmann::json →
   ImGui tree node ricorsivo). Nessuno schema assunto: una chiave nuova
   introdotta da Anthropic appare da sola, senza update di DevDash.

Il livello 2 è la risposta alla domanda "discovery dinamica": tecnicamente
è semplice (directory iteration + parse + tree ricorsivo). Il design vero
sta nei guardrail:

- **NIENTE discovery naive di `~/.claude/*.json`**: la directory contiene
  anche runtime state e segreti (`.credentials.json`, `history.jsonl`,
  cache, stats). Mostrare tutto significherebbe esporre credenziali a
  schermo e rumore.
- **Allowlist di file di configurazione nota** (`settings.json`,
  `settings.local.json`, `keybindings.json`, `.mcp.json` di progetto...) con
  rendering ibrido (semantico dove possibile, generico altrove).
- **Sezione "Other JSON files"** opzionale per i file non in allowlist,
  con **denylist hard** per pattern sensibili/runtime (`.credentials*`,
  `history*`, `*cache*`, `stats*`, dotfile) e mostrata solo on-demand.
- Parse failure = riga visibile con errore, mai silenzio (coerente con la
  filosofia post-refactor di ApplyResult).

## Acceptance (di massima)

1. Le permissions globali e di progetto sono visibili nella config view
   con badge layer.
2. Una chiave inventata aggiunta a settings.json appare nel tree generico
   senza modifiche al codice.
3. `.credentials.json` non è raggiungibile da nessun percorso UI.
