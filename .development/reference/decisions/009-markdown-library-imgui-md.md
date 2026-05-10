# ADR-009: Libreria markdown — imgui_md + MD4C

**Data**: 2026-05-10
**Status**: Accettata

## Contesto

Il pivot a C++/Dear ImGui ([ADR-008](008-pivot-to-cpp-imgui.md)) ha richiesto
di scegliere una libreria per il rendering markdown nativo in ImGui. DevDash
ha il rendering markdown come feature centrale: le viste primarie sono file
`.md` (CLAUDE.md, ADR, spec, .personal/, memory-bank) navigabili come catena
di documenti collegati via `@`-import.

L'ecosistema markdown-per-ImGui è piccolo. Le opzioni concrete:

1. **`enkisoftware/imgui_markdown`** (single-header, zlib, ~1300 ⭐) — la
   scelta "default" della community ImGui. Parser scritto a mano dentro il
   header. Maintainer principale (Doug Binks) attualmente non riesce a stare
   dietro alle PR (confermato da Dario che è in contatto).

2. **`mgerhardy/imgui_markdown` (fork)** — fork attivo che aggiunge tabelle
   e fenced code block. Esiste come PR #43 sull'upstream da 3.5 mesi senza
   merge.

3. **`mekhontsev/imgui_md`** (~200 ⭐, MIT) — bridge piccolo che delega il
   parsing a **MD4C** (parser CommonMark in C, ~2000 ⭐, MIT, attivamente
   mantenuto). Il bridge stesso è inattivo dal 2022.

4. **Scrivere un renderer custom** — partire da MD4C (o cmark) e implementare
   il binding a ImGui da zero.

Durante lo Step 3 del PoC è stato adottato il fork **mgerhardy/imgui_markdown**
per via delle tabelle e dei fenced code block. Allo Step 4 (uso reale su
catene di CLAUDE.md), tre bug pratici sono emersi:

- **Fenced code block**: il language tag (`` ```bash ``) è renderizzato come
  testo, e le righe interne sono classificate come `NORMAL_TEXT` invece di
  `CODE` — perdono lo styling.
- **Inline code in celle di tabella**: i backtick `` ` `` sono renderizzati
  letterali invece di applicare lo styling code.
- **Bold attorno a link**: `**[testo](url)**` rende gli asterischi come
  testo letterale. Limitazione esplicitamente documentata nel sorgente
  upstream (`// we do not support emphasis around links for now`).

Tutti tre sono bug del parser, non del rendering. Il parser di
`imgui_markdown` è scritto a mano dentro il single-header e non delega a
una grammatica esterna.

Il 9 maggio 2026, dopo aver postato i primi due bug come feedback alla PR
#43 di mgerhardy, la scelta è stata rivista.

## Decisione

**Adottare `imgui_md` (mekhontsev) + MD4C** come stack markdown per DevDash.

Architettura:

- **MD4C** è il parser CommonMark (libreria C, MIT, attivamente mantenuta).
  Pulled via CPM da `mity/md4c@release-0.5.3`. Build come libreria statica.
- **imgui_md** è un bridge ~700 righe che ascolta gli eventi di MD4C e li
  traduce in chiamate ImGui. Vendored in `poc/external/imgui_md/` (file
  `imgui_md.h` + `imgui_md.cpp`, MIT, copyright Dmitry Mekhontsev).
- DevDash deriva da `imgui_md` la classe `Rendering::MarkdownRenderer` che
  override `get_font()`, `open_url()`, `get_image()`, `SPAN_CODE`,
  `BLOCK_CODE` per integrare il font system e il color scheme del progetto.

Il preprocessing degli `@`-import (riscrittura come link `claudeimport://`
per la navigazione cross-file) avviene **prima** di passare il testo a
`imgui_md`, ed è fence-aware (la trasformazione viene saltata dentro fenced
code block).

## Rationale

### Perché non `imgui_markdown` upstream

Il parser hand-written non ha CommonMark conformance e i bug osservati
(fenced code block, inline code in tabelle, bold-attorno-a-link) sono
strutturali — risolverli richiede patchare il parser, e il maintainer non
può accettare PR in tempi ragionevoli.

### Perché non il fork mgerhardy

Il fork aggiunge tabelle e fenced code block ma eredita lo stesso parser
hand-written e ne replica i limiti. La PR #43 verso upstream è ferma da
3.5 mesi. Se mai verrà mergiata, l'utente potrà rivalutare; nel frattempo
ogni fix richiede patchare il fork localmente.

### Perché `imgui_md` + MD4C nonostante l'inattività del bridge

Il punto critico è **chi fa il parsing**:

- `imgui_markdown` mette parser e renderer nello stesso header → bug del
  parser sono bug del progetto.
- `imgui_md` separa parser (MD4C, esterno, attivo) e bridge (mekhontsev,
  inattivo) → i bug di parsing che vediamo oggi spariscono perché non
  riguardano il bridge.

L'inattività del bridge è un rischio gestibile:

- Il bridge è ~700 righe, leggibile.
- Le patch necessarie per ImGui 1.92.x sono piccole e ben isolate
  (`PushFont(font, 0.0f)` con size argument; `FontGlobalScale` →
  `FontScaleMain`; `Image(6 args)` → `ImageWithBg`; `CalcWordWrapPositionA`
  → `CalcWordWrapPosition`; `Fonts->TexID` → `Fonts->TexRef.GetTexID()`).
  Tutte già applicate, commentate inline.
- Se il bridge in futuro ostacolasse, è rimpiazzabile con un binding
  custom su MD4C senza buttare il parser.

In altre parole: **il bridge è disposable, il parser no**. La scelta riduce
la superficie di codice di cui DevDash si deve curare quando il markdown si
rompe.

### Perché non scrivere un renderer custom

Considerato. Costa ~700 righe di bridge da scrivere (lo stesso ordine di
magnitudine di `imgui_md`) per fare la stessa cosa che `imgui_md` fa già.
Senza giustificazione concreta sul terreno tecnico (il bridge mekhontsev
non blocca alcuna feature richiesta), riscrivere è inflazione di scope.

Da rivalutare se: (a) il bridge si rompe in modo non-patchabile, (b) servono
feature di rendering che il bridge non offre (es. selezione testo, copia
con styling preservato, scrolling di code block lunghi).

### Coordinamento con Germen

Dario su Germen Pulchrum usa l'upstream `enkisoftware/imgui_markdown` e non
ha incontrato i bug che DevDash ha incontrato (uso meno intenso del rendering
markdown). Nessun lavoro da coordinare oggi. Se in futuro Dario adotterà la
PR #43 o passerà a `imgui_md`, lo segnaliamo.

## Conseguenze

### Pro

- ✅ **Parser CommonMark conforme**: tabelle, fenced code block, inline code
  ovunque funzionano per costruzione, non per patch.
- ✅ **Manutenzione concentrata sul bridge**, dove il codice è piccolo e
  leggibile. Patch ImGui 1.92.x già applicate e isolate.
- ✅ **Ecosistema MD4C maturo**: usato da QGit, Far Manager, vim-markdown
  preview, vari static site generator. Bug del parser vengono risolti
  upstream.
- ✅ **Override pulito** della classe `imgui_md::md` per integrare il font
  system e il color scheme di DevDash.
- ✅ **Tutti i bug osservati su imgui_markdown spariscono**: language tag
  parsing corretto, inline code in tabelle funziona, bold-attorno-a-link
  reso bene.

### Contro

- ❌ **Bridge inattivo dal 2022**: rischio che imminenti API breaks di
  ImGui richiedano patch locali. Mitigazione: monitorare ImGui changelog,
  patch già applicate sono documentate inline nel `.cpp`.
- ❌ **Due dipendenze invece di una**: MD4C + imgui_md vendored. Tradeoff
  accettato.
- ❌ **Limitazione tabelle**: la larghezza delle colonne è definita dal
  contenuto dell'header (limitazione documentata nell'upstream README di
  `imgui_md`). Workaround pratico: header descrittivi nei doc reali (è
  anche buona pratica markdown). Refactor verso `ImGui::BeginTable()`
  nativo è una sessione a sé se mai serve.

### Tech-debt registrato

- `markdown-code-block-styling` — solo colore giallo, niente syntax
  highlighting né monospace; tre opzioni di evoluzione (panel + monospace,
  ImGuiColorTextEdit, tree-sitter).
- `preprocess-imports-indented-fences` — fence indentate (fino a 3 spazi,
  CommonMark) non riconosciute dal preprocessor `@`-import; oggi nessun
  documento reale colpito.

### Limitazioni note (non tech-debt, da ricordare)

- Il flag `m_is_code` della base `imgui_md` non viene settato dalla
  `BLOCK_CODE` di default (solo `SPAN_CODE` lo setta nella nostra override).
  Se un giorno servisse leggerlo da `get_font()` per ritornare un font
  monospace, ricordare di settarlo nel nostro override `BLOCK_CODE` o
  chiamare la base.
- DejaVu Sans è hardcoded come fallback su `/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf`
  per glyph mancanti in IBM Plex Sans (frecce, dingbats). Per il PoC va bene;
  nel progetto vero serve strategia più robusta (font bundled o detection).

## Note per il futuro

### Se PR #43 di mgerhardy viene mergiata su upstream

Da rivalutare *solo* se aggiunge feature che `imgui_md`/MD4C non hanno —
non per tornare a `imgui_markdown` per default. Il problema di partenza
era il parser hand-written, che resta hand-written anche post-PR.

### Se Dario adotta `imgui_md`

Coordinarsi su:
- Patch ImGui 1.92.x (già applicate da DevDash, condividibili).
- Override per font/color scheme: pattern simile, ma ogni progetto la
  personalizza diversamente.
- Eventuali fix al bridge: condividere upstream a mekhontsev se ricostituisce
  attività, o pubblicare patch in fork di entrambi i progetti.

### Sostituire il bridge con MD4C diretto

Solo se il bridge si rompe in modo non-patchabile o blocca una feature
richiesta. È un'opzione di fuga, non un'evoluzione pianificata. MD4C resta
in ogni caso lo strato di parsing.

## Related ADRs

- [ADR-008](008-pivot-to-cpp-imgui.md) — Pivot a C++/Dear ImGui. Questo
  ADR è una sub-decisione di quel pivot.

## Riferimenti

- imgui_md: `https://github.com/mekhontsev/imgui_md` (MIT, Dmitry Mekhontsev).
- MD4C: `https://github.com/mity/md4c` (MIT, Martin Mitas).
- imgui_markdown upstream: `https://github.com/enkisoftware/imgui_markdown`
  (zlib, Doug Binks).
- PR #43 mgerhardy: `https://github.com/enkisoftware/imgui_markdown/pull/43`.
- Memory-bank rilevanti: `2026-05-04-2400-cpp-poc-step4-complete.md`
  (osservazione bug), `2026-05-09-2301-imgui-md-migration.md` (decisione
  e migrazione).
- Commit migrazione: `c643eb2` (refactor su `experiment/imgui-md-migration`),
  merge `959a7c2` su `develop`.
- Codice: `poc/external/imgui_md/`, `poc/src/rendering/markdown_r.{h,cpp}`,
  `poc/external/CMakeLists.txt` (CPM di MD4C, target `ImGui-MD`).
- Limitazioni e patch ImGui 1.92.x: documentate inline in `imgui_md.cpp`.
