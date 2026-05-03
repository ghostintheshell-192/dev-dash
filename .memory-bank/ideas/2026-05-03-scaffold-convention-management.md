---
captured: 2026-05-03
status: open
context: "emerso durante Step 2 del PoC C++/ImGui — al momento di fissare le naming conventions C++ (file rules/coding-standards.md), Valentina ha chiesto se la decisione potesse essere supportata anche dallo scaffolding di DevDash, non solo memorizzata in conversazione"
tags: [scaffold, conventions, language-detection, mutual-understanding, view-of-truth]
---

## Cos'è

Una capability di DevDash che aiuta l'utente a **decidere e applicare le coding conventions** del proprio progetto, integrata nello scaffolding e visibile a runtime.

Tre livelli, in ordine di ROI crescente:

1. **Convention picker nello scaffolding**: dopo che `ScaffoldService` rileva i linguaggi del progetto (già lo fa per i build commands tech-stack-specific), offre un picker di "famiglie di stile" comuni per linguaggio (C++: Google / LLVM / Microsoft / .NET-aligned / custom; C#: Microsoft / StyleCop / .editorconfig-only; Python: PEP8 / Black / Ruff; ecc.). La scelta popola il placeholder `{LANGUAGE_SPECIFIC_STANDARDS}` in `coding-standards.md` e — quando appropriato — genera anche i file di tooling coerenti (`.clang-format`, `.editorconfig`, `pyproject.toml [tool.ruff]`, ecc.).

2. **Catalogo di stili come risorsa first-class**: una tassonomia versionata di style families per linguaggio, con per ciascuna: tabella di convenzioni in markdown (per `coding-standards.md`), file di tooling pronti, e razionale ("perché Google" / "perché LLVM"). Il catalogo è esso stesso parte del resource-model — `Convention` è una risorsa con identità.

3. **Runtime view-of-truth sulle convenzioni**: in DevDash, una vista che mostra il delta tra convenzioni dichiarate e codice reale. Prima diagnosi: "questi N file violano la convenzione del prefisso `_` sui privati". È esattamente il tipo di "infrastructure that makes context more available" che il documento cognitivo di gennaio chiama "60% del 60/30/10".

## Perché merita

- **Si aggancia al wedge di DevDash**: effective-configuration view e runtime view-of-truth sono i due pillar in cui DevDash ha un moat genuino (vedi handoff `2026-04-29-0013-scaffold-realignment-and-future-tracks.md`, sezione Notes — "mutual understanding come north star"). Le convenzioni sono un caso d'uso compatto e concreto per entrambi i pillar in un'unica feature.
- **Già scaffold-ready**: il refactor di Step 1 ha lasciato `{LANGUAGE_SPECIFIC_STANDARDS}` come placeholder esplicito in `coding-standards.md` dello scaffold. Il picker è il modo naturale per popolarlo. Senza picker, ogni utente deve scrivere a mano le tabelle — costo cognitivo alto, output incoerenti, e Claude non sa che decisione è stata presa.
- **Riduce la frizione di onboarding**: oggi, ogni nuovo progetto richiede una piccola conversazione "che convenzioni adottiamo?" (esattamente quella che è successa stasera per il C++ del PoC). Una volta che il picker esiste, la conversazione diventa una scelta di 30 secondi con default sensati.
- **Aggancio al cognitive framework**: dare conventions esplicite e verificabili è un caso d'uso da manuale di "mutual understanding": Claude vede ciò che l'utente ha deciso, l'utente vede come Claude sta applicando la decisione. Non serve un metaframework per ottenerlo — serve infrastruttura.

## Next-step minimo se ripreso

1. **Catalog seed**: scrivere a mano 3-5 style families per i 2-3 linguaggi più probabili (C++, C#, Python). Schema YAML/JSON: `{language, family_name, source_url, conventions_md, tooling_files: [...]}`. Inserire la decisione C++ presa stasera (vedi `.claude/rules/coding-standards.md` sezione "C++ Conventions") come prima entry "C++ / DevDash custom".
2. **ScaffoldService extension**: durante apply-time del scaffold, dopo language detection, mostrare un piccolo picker e applicare la scelta (sostituzione placeholder + scrittura tooling files). Picker UI può vivere in `ProjectInitializationViewModel` (già esiste).
3. **(Differito)** view-of-truth runtime: richiede integrazione con un linter/formatter per linguaggio. Probabilmente uno spec separato. Per ora basta che lo scaffolding popoli correttamente — già metà del valore.

## Riferimenti

- `.claude/rules/coding-standards.md` § "C++ Conventions" — il caso d'uso che ha generato l'idea (decisione presa in conversazione, ora codificata)
- `.development/tech-debt/scaffold-architecture-scripts.md` (e adiacenti) — track di tech-debt scaffold rimanente, di cui questa idea è un'estensione concettuale
- Handoff `2026-04-29-0013-scaffold-realignment-and-future-tracks.md` — note sui "60/30/10" e "mutual understanding"
