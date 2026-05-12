# DevDash - Current Status

*Last updated: 2026-05-10*

## Project Phase

**Phase**: Wedge feature in corso — effective config view implementata (1/3).

Il pivot a **C++20 + Dear ImGui + SDL3 + Vulkan** è validato e lo skeleton
layered (`core/services/ui/platform/app/`) è in produzione. La prima wedge
feature è live: project selector all'avvio + vista effective config con sezioni
per tipo (CLAUDE.md, Rules, Memory, Skills, Agents, MCP, Hooks).

Stato attuale del codebase:

- `app/` — build funzionante. Skeleton layered completo + feature-effective-config-view implementata.
- Architettura decisa in [ADR-010](reference/decisions/010-architecture-design.md)
  + [api-design.md](api-design.md).
- Legacy `.NET 8 + Avalonia` rimosso da `develop` il 2026-05-10. Recover via `git checkout legacy/avalonia-final`.
- `.github/workflows/ci.yml.disabled` — CI .NET disabilitata pre-pivot.
  Da rimpiazzare con workflow CMake/GCC.

## Recent Work

### 2026-05-10: feature/effective-config-view implementata

- **Project selector** all'avvio: `ProjectSelectorPanel` con SDL3 native folder
  dialog (`SDL_ShowOpenFolderDialog`) e validazione path.
- **Effective config view**: `EffectiveConfigPanel` con tabella raggruppata per
  sezione — CLAUDE.md (+ @includes transitivi, max 5 hop), Rules, Memory,
  Skills, Agents, MCP Servers, Hooks. Badge layer (Global/Workspace/Project)
  con colori per ogni riga. Sezioni "on demand" marcate.
- **ConfigFileScanner**: scanner generico directory + resolver transitivo
  degli `@path` in file markdown.
- **SettingsParser**: lettura `settings.json` (nlohmann/json 3.11.3 via CPM)
  per MCP server e hook entries.
- **ConfigResolver**: section-aware, un resolver per tipo.
- **App state machine**: `kSelectingProject` → `kViewingConfig` → back.
- Merge `feature/effective-config-view` → `develop` (`b12126c`).

### 2026-05-10: Pivot documentation + wedge spec + architecture design + legacy cleanup

- **CURRENT-STATUS.md** riallineato (era fermo al 2026-01-01).
- **ADR-008** (pivot stack C++/Dear ImGui, supersede ADR-001) e **ADR-009**
  (libreria markdown `imgui_md` + MD4C). ADR-001 marcato `Superseded by ADR-008`.
- **Wedge feature** formalizzata in 3 spec planned + 1 backlog
  (`feature-effective-config-view`, `feature-scaffold-management`,
  `feature-snapshot-history`, e `backlog/feature-runtime-view-of-truth`).
- **ADR-010 + api-design.md** — split layered del progetto vero
  (`core/services/ui/platform/app/`), 9 decisioni con rationale, `.h` signatures
  per ogni classe del target tree. Pattern callback per `MarkdownRenderer` con
  declaration-order trick, `FontLibrary` come risorsa app-wide, concrete services
  no virtual interfaces, `camelCase` per public struct fields.
- **Legacy `.NET/Avalonia` rimosso da `develop`**: `src/DevDash/`,
  `dev-dash.sln`, `Directory.Build.props`, `Directory.Packages.props`,
  `global.json`, `nuget.config`. Tech-debt `contentcontrol-binding-multisidebar`
  archiviato come `closed` per obsoletizzazione.

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

### Immediato

1. **`feature-scaffold-management`** (parte 2/3 wedge) — vedi spec in `planned/`.
   Mostrare la lista degli scaffold disponibili in `~/.devdash/scaffolds/`,
   applicarli a un progetto, vedere il diff bidirezionale progetto ↔ scaffold.
   Prerequisiti soddisfatti: effective-config-view implementata, `DiffEngine`
   e `ApplyEngine` stub pronti per essere riempiti.

2. **`docs/architecture.md` rewrite** — il file pubblico parla ancora di Avalonia.
   Da aggiornare dopo che la wedge è completa, così descrive cosa esiste davvero.

### Pulizia post-pivot

Cleanup items completati 2026-05-10 (branch `chore/post-pivot-cleanup`):

- ✅ Hook `02-dotnet-format` → `02-clang-format`. Dormiente finché non
  esiste un `.clang-format` al root; quando lo aggiungi, si attiva.
- ✅ `.claude/settings.json` (live config): rimossi
  `Bash(dotnet build:*)`/`Bash(dotnet test:*)`, aggiunti `Bash(cmake:*)`,
  `Bash(ninja:*)`, `Bash(make:*)`, `Bash(ctest:*)`.
- ✅ Hook `04-generate-architecture` + `generate-architecture.sh` +
  `extract-summary.sh`: riscritti per C++/ImGui (FILE_GLOBS array,
  SOURCE_DIRS=poc/src/, generate_project_header riscritto, extract-summary
  con branch C++ per primi `//` block top-of-file/class).
- ✅ `generate-claude-config.sh`: rimossa heredoc-generation di
  `coding-standards.md` (clobberava il file hand-maintained con template
  C#). Il file resta hand-maintained.
- ✅ `.gitignore`: rimossi pattern .NET (Visual Studio, NuGet, `*.dll`,
  `bin/`, `obj/`, ecc.), aggiunti pattern C++/CMake (`build/`, `*.o`,
  `CMakeCache.txt`, `compile_commands.json`).
- ✅ `.claude/rules/workflow.md` Quick Commands: aggiornati a CMake/Ninja.

Pendente:

- 🔲 **`docs/SETUP.md`**: ancora referenzia `src/DevDash` + `dotnet build`.
  Da aggiornare durante il rewrite di `docs/architecture.md` (dopo lo skeleton),
  così descrivono cosa esiste, non cosa è progettato.

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

### Stack runtime (`app/`)

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

### Layout `app/`

Vedi `ARCHITECTURE.md` per il tree dettagliato e ADR-010 per il rationale
della suddivisione `core/services/ui/platform/app/`.

## Blockers / Attention

- **Nessun blocker attivo.**
- **Hook e settings pre-pivot** lentamente invecchiano (vedi Next Steps
  punti 3-8). Non rompono nulla oggi (sono `allow`-pattern, non `deny`).

## Build Status

| Target | Stato |
| ------ | ----- |
| `app/` (C++/CMake, layered) | ✅ Build + runtime su Debian 12 (GCC, Ninja, SDL3/Vulkan/ImGui) |
| Legacy `.NET/Avalonia` | 🗑️ Rimosso da `develop` 2026-05-10; recover via `git checkout legacy/avalonia-final` |
| CI | ⏸️ Disabilitata pre-pivot (`ci.yml.disabled`); da riattivare per CMake |

## Quick Links

- [ADR-008: pivot a C++/Dear ImGui](reference/decisions/008-pivot-to-cpp-imgui.md)
- [ADR-009: libreria markdown imgui_md + MD4C](reference/decisions/009-markdown-library-imgui-md.md)
- [ADR-010: architettura del progetto vero](reference/decisions/010-architecture-design.md)
- [api-design.md: design definitivo](api-design.md) — companion operativo di ADR-010
- [Tech-debt index](tech-debt/README.md)
- [Spec planned/in-progress/implemented](specs/)
- [Memory-bank handoff](../.memory-bank/) — diari di sessione
- Tag legacy: `git checkout legacy/avalonia-final` per recuperare lo stato
  `.NET/Avalonia` pre-pivot.
