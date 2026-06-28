---
captured: 2026-06-28
status: open
context: "Emersa rivedendo le OQ di feature-scaffold-management durante la sessione scaffold-templating: la spec era stata marcata `implemented` con Open Questions ancora aperte."
tags: [process, spec-lifecycle, hooks, quality]
---

# Una spec non passa a `implemented` con Open Questions aperte

## Cos'è

Convenzione di processo mancante: oggi una spec può arrivare a `implemented`
**con Open Questions ancora aperte** (è successo a `feature-scaffold-management`).
Questo racconta una bugia sul processo — "implementato senza chiudere le domande"
— e seppellisce decisioni/lavoro residuo in un documento d'archivio che nessuno
riapre.

Principio da fissare:

1. Una spec passa a `implemented` **solo se non ha OQ aperte** (vanno risolte, o
   estratte in una spec `planned`/`backlog` come lavoro residuo, o promosse ad ADR).
2. Una spec `implemented` **non si tocca più**: è record, non documento vivo.
   Nuove domande/direzioni vanno nei canali vivi (ADR auto-caricati, idea note,
   spec planned), mai aggiunte all'archivio.
3. Se un artefatto `implemented` **va corretto** (errore di processo scoperto
   dopo), la correzione va fatta in modo **tracciato** — un `git revert`, o un
   commit che dichiara esplicitamente "sto correggendo un errore" — non con un
   edit in-place che si mimetizza nel lavoro normale. (Nota meta: in questa
   stessa sessione le OQ di `feature-scaffold-management` sono state svuotate con
   un edit in-place; la via pulita sarebbe stata un revert/commit-correttivo.
   Lasciato così per stavolta, registrato qui come lezione.)

## Perché merita attenzione

Le decisioni "vive" che servono all'implementazione non devono finire sepolte in
spec implemented (dove non si guardano). È lo stesso problema di discoverability
che la sessione modello-ADR ha affrontato per gli ADR — qui sul lifecycle delle
spec.

## Next-step minimo

- Aggiungere un controllo al **lifecycle hook** delle spec
  (`.development/scripts/spec-workflow.py`, invocato da post-merge/pre-commit):
  quando una spec sta per passare a `implemented`, se il body contiene una sezione
  `## Open Questions` non vuota (oltre a un puntatore "nessuna pendente"),
  **avvisare** (o bloccare, da decidere) — costringendo a risolverle o estrarle.
- Documentare i due punti del principio in `.claude/rules/workflow.md` (sezione
  spec lifecycle automation).

## Provenienza

Caso scatenante: le OQ di `feature-scaffold-management` (granularità diff, default
marker, conflict resolution, seed, permessi) risolte e migrate a
`feature-scaffold-templating` il 2026-06-28.
