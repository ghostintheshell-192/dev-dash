---
captured: 2026-06-12
status: parked
context: "branch chore/scaffold-source-of-truth, discussione ADR-013; reperto dogfooding 2026-06-11 (promote finiti su PROVA1)"
tags: [scaffold, ui-overhaul, defaults]
---

# Default scaffold: marker mancante e ordinamento ingannevole

## Cos'è

Nessuno scaffold in `~/.devdash/scaffolds/` ha il marker `.devdash-default`,
quindi `ScaffoldRepository::GetDefault()` ripiega su `_cache[0]`. L'ordinamento
è alfabetico byte-wise (`scaffold_repository.cpp:78`), per cui `PROVA1` (`P` <
`d`) precede `dev-dash-standard` sia come default implicito sia nell'elenco UI
— concausa probabile dei due promote finiti su PROVA1 nel dogfooding, insieme
al dropdown poco leggibile.

## Perché merita attenzione

Il default silenzioso su uno scaffold di prova è una trappola; si incastra col
tema UI già in agenda ("quale scaffold sto usando?").

## Next-step minimo

1. Verificare che l'apply engine NON copi `.devdash-default` nei progetti
   quando applica uno scaffold (probabile esclusione da aggiungere).
2. Aggiungere `.devdash-default` a `rsrc/project-scaffold/` (lo scaffold
   distribuito è anche il default).
3. Valutare ordinamento case-insensitive e, nella UI, evidenziare il default
   e lo scaffold selezionato (si lega ai 4 reperti del dogfooding).
