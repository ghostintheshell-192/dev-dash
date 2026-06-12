---
captured: 2026-06-12
status: parked
context: "branch feature/ui-overhaul, discussione shell sidebar+workspace; nota di Valentina sulla scelta layout"
tags: [ui, diff, resource-model]
---

# Diff come operazione generica (stile WinMerge)

## Cos'è

Oggi il confronto esiste solo nella forma fissa "scaffold selezionato vs
progetto corrente". L'idea: il diff come operazione applicabile a qualunque
coppia di contenuti confrontabili — scaffold ↔ progetto, scaffold ↔ scaffold,
snapshot ↔ progetto, file ↔ file — alla WinMerge: selezioni due cose, avvii
il confronto, si apre un tab di diff.

## Perché merita attenzione

Si incastra col resource model: se ogni risorsa è first-class e navigabile
da sola, il confronto è l'operazione trasversale che le collega. Snapshot
restore, apply e promote diventano tutti casi particolari di "guarda il
delta, poi agisci".

## Next-step minimo

Definire l'interfaccia "confrontabile" (radice + insieme di path relativi)
e fare di `DiffEngine::CompareTrees` il motore unico; la UI è una selezione
A/B + il `FileDiffPanel`/tab già esistente. Primo caso d'uso concreto:
snapshot ↔ progetto dal panel History.
