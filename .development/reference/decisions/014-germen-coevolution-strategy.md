# ADR-014: Strategia di co-evoluzione con Germen Pulchrum

**Data**: 2026-06-14
**Status**: Accepted
**Impact**: high
**Sommario**: Articola la relazione con Germen Pulchrum come co-evoluzione su uno spettro temporale (copia con attribuzione oggi → `git subtree` poi → adozione wholesale tendenziale), abilita la contribuzione bidirezionale attiva (Valentina collaboratrice), e fissa che il modulo grafi nasca su Germen co-sviluppato ma vincolato a essere portabile (solo Dear ImGui, layout/render disaccoppiati); un addendum 2026-09-26 registra che il modulo è nato come libreria autonoma di Dario (ImGuiDot, layout Graphviz) consumata via CPM.

## Contesto

[ADR-008](008-pivot-to-cpp-imgui.md) ha deciso il pivot a C++/Dear ImGui
appoggiandosi a **Germen Pulchrum** (`DPD85/Germen`, di Dario Passet, MIT)
come kickstart, con strategia di consumo: **(A) copia con attribuzione** per
ora, **(C) git subtree** in tasca per dopo — *niente fork, niente submodule*.

Da allora la relazione si è evoluta oltre quanto ADR-008 cattura, e tre fatti
nuovi rendono utile esplicitarne la strategia *a regime*:

1. **La co-evoluzione è già operativa e bidirezionale.** dev-dash consuma il
   fork `DPD85/imgui_md` di Dario via CPM; il pattern d'integrazione markdown
   di dev-dash è stato informato dalla PR #4 di Germen; feedback tecnici
   rimbalzano tra le due codebase (vedi ADR-009 § "Coordinamento con Germen").
2. **Valentina è collaboratrice sul repository Germen** (aggiunta da Dario).
   Il rapporto non è più solo "consumo con attribuzione": è possibile
   **contribuire attivamente** a monte, non solo pescare a valle.
3. **Il modulo grafi non esiste ancora in Germen.** Dario aveva iniziato a
   lavorarci (su richiesta di dev-dash) ma si è arenato; la pagina ufficiale
   conferma "no graphing capabilities". È il pezzo mancante più rilevante per
   la roadmap di dev-dash (code-graph rendering, vedi ADR-008 § strategica e
   `specs/planned/feature-code-graph.md`).

I due stack restano **deliberatamente divergenti**: Germen usa Conan 2.x,
monolite attorno a `Disegnatore`; dev-dash usa CPM puro, architettura layered
(`core/services/ui/platform/app/`, ADR-010). Questa divergenza è il vincolo
che ogni "tirarsi dentro" codice da Germen deve rispettare.

## Decisione

### 1. La relazione è co-evoluzione, non dipendenza — su uno spettro temporale

La strategia di consumo di ADR-008 si articola in tre tappe esplicite:

- **Oggi → copia con attribuzione.** dev-dash ha il proprio stack (CPM) e
  porta da Germen singoli pezzi verificati, ri-stilati alle convenzioni
  dev-dash e attribuiti in header + `THIRD_PARTY_NOTICES.md`. Ad oggi: la
  genesi del renderer (poi rifattorizzato in `platform/`) e la
  `DeletionQueue`.
- **Prossimo → `git subtree` in tasca.** Quando servirà pescare moduli interi
  (non singole classi), si adotta `git subtree` per conservare la storia e la
  possibilità di re-sync bidirezionale.
- **Tendenziale → adozione wholesale.** Se Germen maturerà *tutti* i pezzi
  che servono a dev-dash, dev-dash potrà adottarlo come base GUI così com'è.
  Questa tappa **non era esplicitata** in ADR-008 ed è la principale aggiunta
  di questo ADR.

### 2. Contribuzione bidirezionale attiva

Essendo Valentina collaboratrice su Germen, il flusso non è solo Germen →
dev-dash. Miglioramenti e moduli di interesse comune possono nascere o essere
portati **a monte** su Germen, a beneficio di entrambe le codebase. La scelta
caso-per-caso (scrivere su Germen vs scrivere in dev-dash e fare upstream)
dipende da chi ha più bisogno del pezzo e su che ritmo.

### 3. Il modulo grafi nasce su Germen, co-sviluppato — ma portabile per vincolo

Il modulo grafi (code-graph) **nasce su Germen**, sviluppato insieme a Dario,
e dev-dash lo consuma (`git subtree` quando pronto). Vincolo di design
**non negoziabile** perché dev-dash possa tirarselo dentro senza trascinare
metà di Germen:

- Il modulo dipende **solo da Dear ImGui** (draw-list) e dai dati del grafo.
- **Nessun accoppiamento** a `Disegnatore` globale, a Boost, a Conan, o ad
  altra infrastruttura host di Germen.
- Si articola in due unità nette: **`layout`** (calcolo coordinate, zero
  dipendenze grafiche) + **`render`** (disegno ImGui). Lo stesso
  disaccoppiamento che lo rende portabile è ciò che abilita la tappa di
  "adozione wholesale" senza dolore.

Il *come* del layout (motore Graphviz vs implementazione Sugiyama propria) e
del rendering Mermaid-style resta **fuori scope** di questo ADR: è oggetto
della sessione di design dedicata al thread grafi.

## Conseguenze

### Pro

- ✅ La strategia Germen è ora esplicita end-to-end, non implicita in ADR-008.
- ✅ Il vincolo di portabilità del modulo grafi è scritto *prima* che il
   codice esista — input concreto da portare a Dario nel design.
- ✅ Lo status di collaboratrice apre la contribuzione a monte, non solo il
   consumo a valle.

### Contro

- ❌ Il code-graph di dev-dash dipende dal **ritmo di co-sviluppo** di Germen.
   Mitigazione: il vincolo di portabilità tiene aperta l'opzione di scrivere
   in dev-dash e fare upstream se Germen rallenta.
- ❌ La **divergenza di stack** (Conan vs CPM, versioni vk-bootstrap/Vulkan)
   andrà gestita al momento del primo `git subtree` non banale. Non risolta
   qui: tracciata come nota per quel momento.

## Addendum 2026-09-26: il modulo grafi è nato come libreria autonoma

Il §3 prevedeva un modulo grafi dentro Germen, consumato da dev-dash con
`git subtree`. Nel frattempo Dario ha fatto una scelta migliore: il modulo è
nato come **libreria autonoma**,
[ImGuiDot](https://github.com/DPD85/ImGuiDot), con un proprio repository e un
modulo CPM. Germen la consuma via CPM (branch `funzionalità/diagrammi2`).

Cosa cambia rispetto al testo sopra:

- **Consumo via CPM, non `git subtree`.** dev-dash la consumerà come già fa con
  `DPD85/imgui_md`. Il "Contro" sulla divergenza Conan/CPM non si applica a
  questo modulo: la libreria porta con sé il proprio modulo CPM.
- **Il vincolo di portabilità è rispettato**: ImGuiDot dipende solo da Dear
  ImGui (draw list) e da Graphviz, niente infrastruttura di Germen.
- **La domanda sul layout è chiusa: Graphviz.** ImGuiDot usa Graphviz per
  parsing DOT e layout, e disegna da sé con la draw list. Lo split
  `layout`/`render` del §3 corrisponde a Graphviz/ImGuiDot.
- **La parte di dev-dash** (estrarre il modello dal codice e generare il DOT)
  è definita in
  [`feature-code-graph`](../../specs/planned/feature-code-graph.md) e in
  [ADR-017](017-code-graph-extraction.md). Il confine fra le due parti è il
  testo DOT.
- **La contribuzione bidirezionale (§2) passa da PR su ImGuiDot** dal fork
  `ghostintheshell-192/ImGuiDot`. Le prime due sono state aperte il
  2026-09-26 (fix per GCC 13 e spazio del diagramma nel layout).

## Vedi anche

- [ADR-008](008-pivot-to-cpp-imgui.md) — pivot a C++/ImGui, strategia di
  consumo Germen originale (§ "kickstart Germen + co-evoluzione con Dario").
- [ADR-009](009-markdown-library-imgui-md.md) § "Coordinamento con Germen".
- [ADR-010](010-architecture-design.md) — architettura layered di dev-dash
  (il vincolo di disaccoppiamento del modulo grafi).
- `app/THIRD_PARTY_NOTICES.md` — attribuzione operativa del codice portato.
- `.development/specs/planned/feature-code-graph.md` — spec del code-graph.
- [ADR-017](017-code-graph-extraction.md) — estrazione del code graph.
