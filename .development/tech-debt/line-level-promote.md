---
type: enhancement
priority: medium
status: open
discovered: 2026-05-10
closed: null
related: []
related_decision: null
report: null
---

# Promote e Apply operano solo sull'intero file

## Problem

Promote (progetto → scaffold) e Apply (scaffold → progetto, US-2) lavorano
a granularità di file: o si copia tutto il file o non si copia nulla.

Quando un file è "Modified", l'utente può vedere riga per riga cosa è cambiato
nel diff viewer — ma non può scegliere di promuovere solo alcune righe/hunks
e ignorare il resto. In pratica, se il file ha 5 differenze e l'utente ne vuole
3, deve:

1. Aprire il file nel documento viewer
2. Editarlo manualmente prima di promuovere

Questo vanifica in parte l'utilità del diff viewer per file con modifiche miste
(alcune buone da propagare, altre no).

## Desired Behaviour

Nel diff viewer, l'utente può selezionare singoli **hunk** (blocchi contigui
di righe cambiate) e promuoverli/applicarli individualmente, lasciando il
resto invariato. Equivalente a `git add -p` (interactive patch staging) ma
con UI grafica.

## Technical Notes

- Richiede di raggruppare le `DiffLine` in hunk contigui (sequenze di
  kAdded/kRemoved senza kContext di separazione).
- Ogni hunk avrebbe un checkbox nel diff viewer.
- "Promote selected hunks" costruisce un file risultante applicando solo
  gli hunk selezionati al file sorgente.
- Complessità non banale: la costruzione del file finale da hunk parziali
  richiede attenzione agli offset di riga.

## Workaround

Usare il document viewer per editare manualmente il file prima di promuoverlo.
