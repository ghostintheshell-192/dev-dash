---
type: feature
priority: should-have
status: backlog
category: core
related: [feature-effective-config-view]
depends_on: [feature-effective-config-view]
created: 2026-05-10
---

# Runtime View-of-Truth

> **Estensione futura della wedge feature.** Complementare a
> `feature-effective-config-view.md` (statico): mentre quella mostra la
> config che Claude *userà* alla prossima sessione, questa mostra la config
> che Claude *ha effettivamente caricato* in una sessione reale. Backlog,
> non in MVP.

## Summary

Confronto tra:
- la **effective configuration risolta** (parte 1 della wedge),
- e cosa Claude Code **ha effettivamente letto / ignorato / re-caricato**
  durante una sessione vera, ricostruito dai transcript e log in
  `~/.claude/projects/<hash>/`.

Lo scopo è diagnostica: rilevare drift tra "cosa la config dice" e "cosa
Claude vede e usa". Esempi di drift osservabili:

- Una regola in `CLAUDE.md` che secondo la config dovrebbe essere caricata,
  ma che nei transcript non risulta mai citata o seguita.
- Un agent che la config dichiara disponibile, ma che non compare mai
  nelle invocazioni reali.
- Re-read patologici di file (es. PRISM riporta "6738% CLAUDE.md re-read
  cost").
- Override silenziosi di rules a metà sessione.

## Background

`.personal/business/analysis/analysis.md` cita questo come secondo pillar
non coperto da nessun competitor:

> "**Runtime view-of-truth** (what Claude actually loaded vs. what's
> configured) — also unaddressed — this is a defensible differentiation"

PRISM (uno dei competitor osservati) si è specificamente posizionato su
questa diagnosi ("rules silently ignored mid-session") — segnale che il
problema è reale e percepito.

## Perché in backlog e non in MVP

- **Dipende da reverse-engineering** dei formati log/transcript di Claude
  Code, che possono cambiare con le versioni.
- **Costo di parsing** non triviale: i transcript jsonl possono essere
  grandi e densi.
- **Valore prima dimostrato**: la parte 1 della wedge (effective config
  view) può essere validata e shippata prima, e *poi* l'utente capisce
  meglio cosa cercare in runtime.
- **Race window stretta**: l'analisi avverte che Anthropic potrebbe
  shippare native diagnostics in tempi rapidi. Vale la pena monitorare
  prima di investirci.

## Open Questions

- Formato dei log/transcript Claude Code: stabile? Versionato? Retro-compat?
- Quali diagnostiche sono utili davvero per l'utente power-user e
  quali sono rumore?
- Privacy: il transcript contiene il prompt e la risposta complete; come
  presentarlo senza esporre contenuti sensibili nella UI?

## Related

- `feature-effective-config-view.md`: complementare statica.
- `.personal/business/analysis/analysis.md`: framing strategico.
