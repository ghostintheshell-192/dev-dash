---
type: bug
priority: low
status: open
discovered: 2026-06-10
related: []
related_decision: null
report: null
---

# ConfigFileScanner: @include verso directory accettato e poi fallisce in silenzio

## Problem

`CollectAtIncludes` (`config_file_scanner.cpp:71-80`) valida i riferimenti
`@path` con `std::filesystem::exists()`, che è vero anche per le directory.
Un `@include` che punta a una directory (caso reale trovato:
`@/data/documenti/Vault@Claude/` dentro `~/.claude/vault-guide.md`) viene
quindi aggiunto a `result` come nodo incluso; la ricorsione su di esso apre
un `ifstream` che fallisce e ritorna senza segnalare nulla.

Effetto: la effective config view mostra la directory come documento
incluso, ma aprirla non mostra contenuto. Nessun errore per l'utente.

## Possible Solutions

- **Option A**: aggiungere `is_regular_file()` al check — le directory
  vengono semplicemente ignorate (fix a una riga).
- **Option B**: Option A + modellare gli include irrisolvibili come nodo
  "broken include" visibile in UI, invece di scartarli — coerente con la
  filosofia "mai fallire in silenzio" (cfr. ApplyResult, refactor
  2026-05-14).

## Recommended Approach

Option A subito alla prossima sessione di codice (banale). Option B da
valutare insieme alla spec backlog `feature-global-settings-coverage`, che
introduce lo stesso principio (parse failure = riga visibile, mai
silenzio).

## Related Documentation

- **Code Locations**: `app/src/services/config_file_scanner.cpp:71-80`
- `.development/specs/backlog/feature-global-settings-coverage.md`
