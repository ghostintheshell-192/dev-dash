# DevDash - Current Status

*Last updated: 2026-05-10*

## Project Phase

**Phase**: PoC C++/Dear ImGui completato — pre-avvio del progetto vero.

Il pivot dallo stack originale (.NET 8 + Avalonia, [ADR-001](reference/decisions/001-stack-tecnologico.md))
a **C++20 + Dear ImGui + SDL3 + Vulkan** è validato. Vedi
[ADR-008](reference/decisions/008-pivot-to-cpp-imgui.md) per la decisione
e il rationale completi.

Stato attuale del codebase:

- `poc/` — PoC C++/ImGui funzionalmente completo (4 step + migrazione
  libreria markdown). Render markdown con navigazione cross-file via
  `@`-import, font system IBM Plex Sans + DejaVu fallback, Vulkan via
  vk-bootstrap, SDL3 windowing.
- `src/DevDash/` — codebase legacy `.NET 8 + Avalonia`, preservato sotto
  il tag `legacy/avalonia-final`. Da rimuovere da `develop` con branch
  dedicato quando il progetto vero parte.
- `.github/workflows/ci.yml.disabled` — CI .NET disabilitata pre-pivot.
  Da rimpiazzare con workflow CMake/GCC.

## Recent Work

### 2026-05-10: Documentazione del pivot (questa sessione)

- `CURRENT-STATUS.md` riallineato (era fermo al 2026-01-01).
- Scritti **ADR-008** (pivot dello stack a C++/Dear ImGui, supersede ADR-001)
  e **ADR-009** (scelta libreria markdown: `imgui_md` + MD4C).
- ADR-001 marcato `Superseded by ADR-008`; index del README aggiornato.

### 2026-05-09: Migrazione libreria markdown — `imgui_markdown` → `imgui_md`

- Pivot dalla `enkisoftware/imgui_markdown` (parser hand-written) a
  `mekhontsev/imgui_md` + **MD4C** (parser CommonMark esterno, MIT,
  attivamente mantenuto). Tutti i bug osservati durante lo Step 4
  (fenced code block, inline code in tabelle, bold-attorno-a-link)
  spariscono perché non sono bug del rendering ma del parser.
- Nuova architettura: `Rendering::MarkdownRenderer` deriva da `imgui_md`,
  override per font system e color scheme. Pannelli gestiti da `Renderer`
  con coda `_pendingPanels` per i `claudeimport://` link cliccati.
- Patch ImGui 1.92.x al bridge vendored (`PushFont` size argument,
  `FontGlobalScale` → `FontScaleMain`, `Image` → `ImageWithBg`,
  `CalcWordWrapPositionA` → `CalcWordWrapPosition`,
  `Fonts->TexID` → `Fonts->TexRef.GetTexID()`).
- `PreprocessImports()` reso fence-aware (la trasformazione `@import` →
  `claudeimport://` viene saltata dentro fenced code block).
- Tech-debt registrati: `markdown-code-block-styling` (no syntax highlighting
  né monospace), `preprocess-imports-indented-fences` (fence indentate
  CommonMark non riconosciute, oggi nessun documento reale colpito).
- Branch `experiment/imgui-md-migration` mergiato (`959a7c2`).

### 2026-05-04: PoC Step 4 — pivot validato

- **Step 4a**: font system IBM Plex Sans (Regular / Italic / Bold + H1/H2/H3
  bold a 30/22.5/17.55px) caricato da `poc/assets/fonts/` via
  `SDL_GetBasePath()`. CMake POST_BUILD copia `assets/` accanto all'eseguibile.
- **Step 4b**: navigazione `CLAUDE.md` con due pannelli iniziali (globale
  + progetto), `PreprocessImports()` riscrive `@path` come `claudeimport://`
  e click apre nuovo pannello dock. DejaVu Sans aggiunto in merge mode su
  ogni face IBM Plex come fallback per glyph mancanti (frecce, dingbats).
- **PoC dichiarato funzionalmente completo**: tutto quello che doveva
  validare lo ha validato. Le feature non coperte (file watching, kanban,
  scaffold automation, layout multi-window) sono feature del progetto
  vero, non requisiti del PoC.
- Mergiato su `develop` (`bbcac65`).

### 2026-05-03: PoC Step 2 + Step 3 — porting Germen e markdown

- **Step 2**: porting di `Germen Pulchrum/Disegnatore.cpp` →
  `poc/src/renderer.cpp`. Tradotto it→en, stripping di feature non necessarie
  (4-stack font, sistema temi, DPI scaling, i18n, status bar custom),
  mantenuti init Vulkan via vk-bootstrap, swapchain, SDL3 window, ImGui
  frame loop. Obiettivo Step 2 raggiunto: si apre finestra ImGui con
  DemoWindow funzionante.
- **Step 3**: integrato `imgui_markdown.h` come single-header in
  `poc/external/imgui_markdown/` (poi rimpiazzato con `imgui_md` il
  9 maggio).

### 2026-04-27: PoC Step 1 + cleanup pre-pivot

- Skeleton C++/ImGui scaffoldato in `poc/` (CMake ≥3.28, CPM, SDL3 +
  Dear ImGui + vk-bootstrap, smoke-test build).
- CI .NET disabilitata (`.github/workflows/ci.yml` →`ci.yml.disabled`)
  per togliere il badge rosso GitHub Actions sul codice Avalonia
  condannato.
- Skeleton code-graph rimesso in `planned` con disclaimer pre-pivot.
- Strategia di consumo Germen decisa: **(A) copia con attribuzione** per
  ora, **(C) git subtree** in tasca per dopo se Germen evolverà attivamente.
  Niente fork, dipendenze GUI via CPM (no Conan).

### Sessioni pre-pivot

Le sessioni 2025-12 / 2026-01 hanno costruito la v0.2.x in Avalonia
(workspace portabile, sidebar multi-pannello, detection `.rules/` /
`.memory-bank/`, scaffold service). Codebase preservato sotto
`legacy/avalonia-final`. Vedi memory-bank per i dettagli.

## Next Steps

### Immediato (prossima sessione)

1. **Iniziare il progetto vero**. Decisione architettonica grossa:
   - Promozione `poc/` → `app/` (o equivalente).
   - Rimozione di `src/DevDash/` da `develop` (preservando il tag
     `legacy/avalonia-final`).
   - Struttura cartelle definitiva, equivalente MVVM in C++ (separazione
     domain / view / service o pattern alternativo da decidere).
   - CMake definitivo: targets, install rules, packaging, eventuale
     workflow CI Linux.
   - Potrebbe meritare un **ADR-010** sulla struttura del progetto vero.

### Pulizia post-pivot (bassa-media priorità, da fare durante avvio progetto vero)

2. **Rimuovere `src/DevDash/` da `develop`** in branch dedicato dopo aver
   verificato che il tag `legacy/avalonia-final` punti allo stato corretto.

3. **Sostituire hook `02-dotnet-format`** con `clang-format` (o equivalente
   per il PoC Linux-only).

4. **Aggiornare `.claude/settings.json` permissions**: rimuovere
   `Bash(dotnet build:*)` e `Bash(dotnet test:*)`, aggiungere `Bash(cmake:*)`,
   `Bash(ninja:*)`, `Bash(make:*)`.

5. **Aggiornare hook `04-generate-architecture`** — il pattern grep oggi è
   `\.(cs|py|ts|rs)$`, va esteso ad `cpp|hpp|h` perché altrimenti
   `ARCHITECTURE.md` non si rigenera mai dopo modifiche al codice C++.

6. **Aggiornare `coding-standards.md`** auto-generato: lo script
   `generate-claude-config.sh` ha template C#-specifico hard-coded. Da
   estendere per C++ o svuotare il file finché non c'è un template
   equivalente.

### Verifica reattiva (solo se serve)

7. **Layout di tabelle complesse e code block**: oggi solo test minimo. Se
   un giorno una tabella o un code block del progetto reale appare strano,
   riguardare i tech-debt:
   - `markdown-code-block-styling` — solo colore, niente syntax highlighting.
   - `preprocess-imports-indented-fences` — fence indentate CommonMark non
     riconosciute.
   - Limitazione tabelle imgui_md: larghezza colonne dipende da contenuto
     header (workaround: header descrittivi nei doc reali).

## Current Architecture

Vedi `ARCHITECTURE.md` per il tree completo (auto-generato).

### Stack runtime (PoC `poc/`)

| Componente | Versione | Note |
| ---------- | -------- | ---- |
| C++ | 20 | `CMAKE_CXX_STANDARD 20`, no extensions, GCC su Linux |
| Dear ImGui | 1.92.6-docking | Pinned via CPM; config locale `external/imgui-config.h` |
| SDL3 | 3.2.20 | Built from source via CPM (Debian 12 non ha `libsdl3-dev`) |
| Vulkan | 1.3.x | `find_package(Vulkan REQUIRED)` |
| vk-bootstrap | 1.3.302 | Pinned al 1.3.x finché host non ha header Vulkan 1.4 |
| MD4C | 0.5.3 | Parser CommonMark, statico, via CPM |
| imgui_md | vendored (mekhontsev) | Bridge MD4C → ImGui, MIT, in `external/imgui_md/` |
| CMake | ≥3.28 | + Ninja generator |

### Layout `poc/`

```
poc/
├── CMakeLists.txt
├── CMakePresets.json
├── cmake/                           # compiler dispatch + CPM bootstrap
├── external/
│   ├── CMakeLists.txt              # CPM dependencies + ImGui static libs
│   ├── imgui-config.h              # IMGUI_USER_CONFIG locale
│   └── imgui_md/                   # vendored bridge
├── src/
│   ├── main.cpp
│   ├── renderer.{h,cpp}            # Vulkan + SDL3 + ImGui frame loop
│   ├── deletion_queue.h
│   └── rendering/
│       └── markdown_r.{h,cpp}      # MarkdownRenderer : imgui_md
├── assets/
│   ├── fonts/                      # IBM Plex Sans (SIL OFL 1.1)
│   └── ...
└── THIRD_PARTY_NOTICES.md
```

### Patterns in uso (PoC)

- **Bridge override**: `Rendering::MarkdownRenderer` deriva da `imgui_md::md`
  e override `get_font()`, `open_url()`, `get_image()`, `SPAN_CODE`, `BLOCK_CODE`.
- **Lifecycle dei pannelli markdown**: gestita da `Renderer` (`OpenPanel`,
  `RenderMarkdownWindow`, `_panels`, `_pendingPanels`). `MarkdownRenderer`
  riceve `_pendingPanels` per `&` e ci pusha solo i path dei `claudeimport://`
  link cliccati. Separation of concerns clean.
- **Nested type private**: `MarkdownPanel` come nested type di `Renderer` —
  segnala dettaglio implementativo.

## Blockers / Attention

- **Nessun blocker attivo.**
- **Stack legacy ancora presente** in `src/DevDash/` (non più builda,
  ma ingombra il tree). Da rimuovere all'avvio del progetto vero.
- **Hook e settings pre-pivot** lentamente invecchiano (vedi Next Steps
  punti 3-6). Non rompono nulla oggi (sono `allow`-pattern, non `deny`).

## Build Status

| Target | Stato |
| ------ | ----- |
| `poc/` (C++/CMake) | ✅ Compila e linka su Debian 12 (GCC, Ninja) |
| `poc/` runtime | ✅ Apre finestra Vulkan/SDL3 con pannelli markdown navigabili |
| `src/DevDash/` (.NET/Avalonia) | ⚠️ Non più rilevante (legacy, snapshot in `legacy/avalonia-final`) |
| CI | ⏸️ Disabilitata pre-pivot (`ci.yml.disabled`); da riattivare per CMake |

## Quick Links

- [ADR-008: pivot a C++/Dear ImGui](reference/decisions/008-pivot-to-cpp-imgui.md)
- [ADR-009: libreria markdown imgui_md + MD4C](reference/decisions/009-markdown-library-imgui-md.md)
- [Tech-debt index](tech-debt/README.md)
- [Spec planned/in-progress/implemented](specs/)
- [Memory-bank handoff](../.memory-bank/) — diari di sessione
- Tag legacy: `git checkout legacy/avalonia-final` per ispezionare lo stato
  pre-pivot.
