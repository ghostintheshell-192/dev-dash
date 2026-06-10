# DevDash - Current Status

*Last updated: 2026-06-10*

## Project Phase

**Phase**: Wedge completa (3/3) — effective config view, scaffold management
e snapshot & history implementate e mergiate in `develop`.

Il pivot a **C++20 + Dear ImGui + SDL3 + Vulkan** è validato e lo skeleton
layered (`core/services/ui/platform/app/`) è in produzione. Le tre wedge
feature sono live: project selector + effective config view, scaffold
list/diff/apply, snapshot save/restore con History panel e autosnapshot
pre-apply. Flow apply/restore testato end-to-end (dogfooding sul repo stesso,
2026-05-14).

Stato attuale del codebase:

- `app/` — build funzionante. Skeleton layered completo + 3 wedge feature.
- `poc/` — **rimosso** il 2026-05-12 (PoC concluso; font migrati in
  `app/assets/fonts/`).
- Architettura decisa in [ADR-010](reference/decisions/010-architecture-design.md)
  + [api-design.md](api-design.md).
- `imgui_md` consumato via CPM dal fork `DPD85/imgui_md` (override dev-only
  `SOURCE_DIR /data/repos/imgui_md` finché le PR del fork non sono stabili).
- Legacy `.NET 8 + Avalonia` rimosso da `develop` il 2026-05-10. Recover via `git checkout legacy/avalonia-final`.
- `.github/workflows/ci.yml.disabled` — CI .NET disabilitata pre-pivot.
  Da rimpiazzare con workflow CMake/GCC.

## Recent Work

### 2026-06-10: Spec workflow fix + sync documentazione

- **Root cause del tech-debt `spec-workflow-hook-silent-fail`** trovata e
  fixata: mismatch di naming branch→spec (`feature/x` → cercava `x.md`, i
  file sono `feature-x.md`) + regex status su markdown invece che YAML
  frontmatter. Fix in `spec-workflow.py` (fallback prefissato univoco +
  subn YAML-first). Tech-debt chiuso.
- **Le 3 spec wedge spostate in `specs/implemented/`** con frontmatter
  allineato.
- **Nuovo tech-debt `project-githooks-not-active`** (medium): scoperto che
  `core.hooksPath` globale punta agli hook workspace, quindi `.githooks/`
  di progetto — inclusa la branch protection — non gira affatto su questo
  clone.
- `app/external/CMakeLists.txt`: adottate le option `BUILD_IMGUI OFF` /
  `BUILD_MD4C OFF` dell'interfaccia CMakeLists del fork imgui_md (PR2).

### 2026-05-14: Refactor snapshot-history + merge + third-party notices

- **4 refactor commit** dalla code review di `feature/snapshot-history`:
  dedupe `MakeProjectSlug` in `core/project.h`, `originatingAction` come
  `std::optional`, `ApplyEngine::Apply` → `ApplyResult {applied; skipped;
  failed;}` con status message ricco in UI, errori per-file di
  `CopyConfigFiles` loggati su stderr.
- **Merge `feature/snapshot-history` → `develop`** (`abb0abc`); test
  end-to-end apply/restore riuscito.
- **Refresh `app/THIRD_PARTY_NOTICES.md`**: sezione imgui_md (upstream +
  fork DPD85), aggiunte MD4C e nlohmann/json (`3afeb02`).
- Pulizia dei 14 branch locali mergiati; restano `develop` e `main`.
- Diagnosi warning lock `.git/config`: bind mount RO del sandbox Claude
  Code, **cosmetico e atteso** — non investigare più.

### 2026-05-12: snapshot-history + scaffold apply + PR2 fork + rimozione poc/

- **Feature snapshot-history implementata** (`727d2e3`): SnapshotService
  (save explicit/auto, list, restore, pruning), History panel, wiring in
  EffectiveConfigPanel.
- **Apply path scaffold-management completato** (`a7c6359`): bottone
  "Apply..." in ScaffoldDiffPanel via `ApplyEngine` + autosnapshot
  pre-apply. Chiude la parte 2/3 della wedge.
- **PR2 sul fork `DPD85/imgui_md`** (CMakeLists standalone) aperta; PR1
  (word wrap) mergiata da Dario il 2026-05-11. Migrazione
  vendoring → CPM in `app/external/CMakeLists.txt`.
- **`poc/` rimosso** (22 file); font IBM Plex migrati in
  `app/assets/fonts/`.

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

1. **Chiudere il loop sulla PR2 imgui_md** — in attesa di Dario sulla
   controproposta (`IMGUI_TARGET`/`MD4C_TARGET` cache vars + alias MD4C).
   Al merge: sostituire l'override dev-only `SOURCE_DIR /data/repos/imgui_md`
   con `GITHUB_REPOSITORY DPD85/imgui_md` + `GIT_TAG <SHA>` in
   `app/external/CMakeLists.txt` (il blocco commentato "Stable form" fa da
   reminder).

2. **Primo test target del progetto** — sblocca i test unit di
   `SnapshotService` (fixture su tmpdir: save/list/restore/prune), il modulo
   più testabile del codebase.

3. **UX/UI snapshot-history + scaffold-manage** — funzionante sopra le
   primitive ma "tutta da rifare" (Valentina, 2026-05-14). Non prioritaria
   finché il progetto resta single-user.

4. **`docs/architecture.md` + `docs/SETUP.md` rewrite** — i file pubblici
   parlano ancora di Avalonia/.NET. La wedge è completa: ora possono
   descrivere cosa esiste davvero.

5. **Tech-debt `project-githooks-not-active`** — decidere la strategia
   hooksPath (hook progetto vs workspace); finché aperto, la branch
   protection su `main`/`develop` è solo convenzionale.

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
- **Branch protection non effettiva** su questo clone: gli hook di progetto
  `.githooks/` sono bypassati dal `core.hooksPath` globale (vedi tech-debt
  `project-githooks-not-active`). Attenzione ai commit diretti su
  `main`/`develop`.
- **Warning lock `.git/config`** durante operazioni git: cosmetici, causati
  dai bind mount RO del sandbox Claude Code. Non investigare.

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
