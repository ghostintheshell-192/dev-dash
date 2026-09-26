# DevDash - Current Status

*Last updated: 2026-09-26*

## Project Phase

**Phase**: Wedge completa + release readiness — in attesa del primo tag
`v0.1.0`. Prossimo lavoro di prodotto: scaffold templating (ADR-015) e,
sul fronte grafi, la collaborazione con Dario su ImGuiDot.

Il pivot a **C++20 + Dear ImGui + SDL3 + Vulkan** è validato e in produzione.
Le tre wedge feature (effective config view, scaffold management, snapshot &
history) sono live dentro una shell a workspace (top bar, sidebar, dockspace,
status bar) con tema centralizzato "Grafite & Ambra". Il progetto è
installabile, testato (Catch2) e ha CI + release workflow su GitHub Actions.

Stato attuale del codebase:

- `app/` — build funzionante, layered (`core/services/ui/platform/app/`,
  ADR-010). `services/` è anche libreria statica `dev-dash-services`, linkata
  dai test senza stack grafico.
- `app/tests/` — Catch2 v3: `SnapshotService`, `ApplyEngine`.
- Versione `0.1.0` in `app/CMakeLists.txt`; nessun tag `v*` ancora creato.
- `imgui_md` consumato via CPM dal fork `DPD85/imgui_md`, pinnato al commit
  `11832f4` (= tag `v1.0.0`).
- Automazione a due livelli, agnostica rispetto allo stack (ADR-012):
  hook in `.githooks/` (documentati in `.githooks/README.md`) + entry point
  in `.development/automation/`. Attivazione per clone: `bootstrap.sh`.
- Scaffold: fonte di verità `rsrc/project-scaffold/` (ADR-013); templating
  (ADR-015) e versionamento git (ADR-016) decisi, non ancora implementati.
- Legacy `.NET 8 + Avalonia` rimosso da `develop` il 2026-05-10. Recover via
  `git checkout legacy/avalonia-final`.

## Recent Work

### 2026-09-26: Riallineamento config + ImGuiDot upstream

- **Config riallineata con raid-sandbox** (che ne era un fork): skill
  `session-handoff` (Next in ordine di priorità, rimando all'handoff che
  porta la lista), `LC_ALL=C sort` in `generate-architecture.sh`, indice
  tech-debt senza issue resolved/dropped, hook SessionStart/End più robusti,
  regole preflight + "Keeping CURRENT-STATUS.md current", nuovo
  `.githooks/README.md`, pulizia residui C#/Avalonia in `.gitattributes`.
- **ImGuiDot** (`DPD85/ImGuiDot`, Graphviz → ImGui draw list, v2.0.0): è la
  libreria diagrammi di Dario, già integrata nel branch
  `funzionalità/diagrammi2` di Germen. Fork `ghostintheshell-192/ImGuiDot`,
  due PR aperte upstream: `fix/gcc13-sqrt` (`std::sqrtf` non esiste in
  libstdc++ < 14) e `fix/diagram-layout-size` (`Draw()` non riservava spazio
  nel layout). Proposta a Dario sui colori di default (tema scuro) in attesa
  di risposta.

### 2026-08-29/30: Derived docs affidabili

- `post-merge` rigenera i documenti derivati (git non esegue pre-commit sui
  merge senza conflitti); merge driver `generated` per ARCHITECTURE.md e
  INDEX.md; generatori deterministici e in grado di fare bootstrap.
- INDEX.md come mappa (non storia), Layer Overview di ARCHITECTURE.md
  riscritta, CLAUDE.md senza enumerazione delle regole.
- Archiviazione tech-debt con la data di chiusura dal frontmatter.

### 2026-07-25: Audit dello scaffold

- Fix emersi dall'audit di `rsrc/project-scaffold/`: generatori doc
  idempotenti, glob di `generate-architecture` valutato correttamente,
  l'hook di archiviazione non perde più la cancellazione, comando `/handoff`
  delegato alla skill, idea notes spostate in `.memory-bank/ideas/`.

### 2026-06-28/29: ADR scaffold

- **ADR-015** (templating guidato da manifest) e **ADR-016** (versionamento
  git gestito degli scaffold); spec `feature-scaffold-templating` in
  `planned/`. Modello ADR allineato fra progetto e scaffold.

### 2026-06-10/14: Automazione agnostica, UI overhaul, release readiness

- **`feature-agnostic-automation`** (ADR-012): entry point standard +
  hook orchestratori; il tech-debt `project-githooks-not-active` è chiuso.
- **ADR-013**: `rsrc/project-scaffold/` fonte di verità, symlink in dev.
- **`feature-ui-overhaul`**: tema "Grafite & Ambra" (`ui/theme`), shell a
  workspace (`ui/shell`, `ui/sidebar`, `StatusSink`), compare view
  ridisegnata, diff leggibile, DPI scaling, marker dello scaffold di default.
- **`feature-release-readiness`** (ADR-011): Catch2, versione + `version.h`,
  asset a due tentativi con DejaVu bundled, install rules + `.desktop`,
  CPack TGZ, `ci.yml` + `release.yml`, `CHANGELOG.md`, docs utente riscritte.
- **ADR-014**: co-evoluzione con Germen, contribuzione attiva upstream.

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
  `BUILD_MD4C OFF` dell'interfaccia CMakeLists del fork imgui_md (PR2,
  mergiata da Dario) e rimosso l'override dev-only `SOURCE_DIR`: ora
  pinnato a `DPD85/imgui_md` main (`11832f4`).

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

1. **Primo tag `v0.1.0`** — l'acceptance 3 di `feature-release-readiness`
   (release GitHub con tarball) è l'unica non ancora verificata: i workflow
   si collaudano solo al primo tag su `main`.

2. **`feature-scaffold-templating`** (planned, ADR-015) — prossima feature
   di prodotto.

3. **Diagramma del progetto aperto** — obiettivo principale del thread
   grafi (spec in backlog: `feature-code-graph`, da riscrivere per lo stack
   attuale). Passa da ImGuiDot: seguire le PR aperte e la risposta di Dario
   sui colori, poi definire cosa serve di più alla libreria (cluster, errori
   di parsing esposti, interattività).

### Più avanti

- **UX/UI snapshot-history** — funzionante ma "tutta da rifare"
  (Valentina, 2026-05-14); non prioritaria finché single-user.
- **Dock layout persistence** (`io.IniFilename`) non ancora abilitata.

### Verifica reattiva (solo se serve)

- **Layout di tabelle complesse e code block**: se un documento reale appare
  strano, riguardare i tech-debt `markdown-code-block-styling` e
  `preprocess-imports-indented-fences`, e la limitazione di imgui_md sulla
  larghezza delle colonne (dipende dal contenuto dell'header).

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
| imgui_md | fork `DPD85/imgui_md` @ `11832f4` (v1.0.0) | Bridge MD4C → ImGui, MIT, via CPM |
| nlohmann/json | 3.11.3 | via CPM |
| Catch2 | v3 | Test (`app/tests/`), via CPM |
| CMake | ≥3.28 | + Ninja generator |

### Layout `app/`

Vedi `ARCHITECTURE.md` per il tree dettagliato e ADR-010 per il rationale
della suddivisione `core/services/ui/platform/app/`.

## Blockers / Attention

- **Nessun blocker attivo.**
- **Hook per clone**: senza `bash .development/automation/bootstrap.sh` la
  branch protection su `main`/`develop` e gli hook di progetto non girano.
- **Handoff solo locali**: `.memory-bank/sessions/` non è tracciato, quindi
  le sessioni cloud non vedono i diari. La continuità passa da questo file.
- **Warning lock `.git/config`** durante operazioni git: cosmetici, causati
  dai bind mount RO del sandbox Claude Code. Non investigare.

## Build Status

| Target | Stato |
| ------ | ----- |
| `app/` (C++/CMake, layered) | ✅ Build + runtime su Debian (GCC, Ninja, SDL3/Vulkan/ImGui) |
| Test (`dev-dash-tests`) | ✅ Catch2 via `.development/automation/test.sh` |
| CI | ✅ `ci.yml` (build + test, warnings-as-errors) su push/PR a develop/main |
| Release | ⏳ `release.yml` pronto, mai esercitato (nessun tag `v*`) |
| Legacy `.NET/Avalonia` | 🗑️ Rimosso; recover via `git checkout legacy/avalonia-final` |

## Quick Links

- [ADR-008: pivot a C++/Dear ImGui](reference/decisions/008-pivot-to-cpp-imgui.md)
- [ADR-009: libreria markdown imgui_md + MD4C](reference/decisions/009-markdown-library-imgui-md.md)
- [ADR-010: architettura del progetto vero](reference/decisions/010-architecture-design.md)
- [Indice ADR](reference/decisions/README.md) — fino ad ADR-016
- [api-design.md: design definitivo](api-design.md) — companion operativo di ADR-010
- [Tech-debt index](tech-debt/README.md)
- [Spec planned/in-progress/implemented](specs/)
- [Memory-bank handoff](../.memory-bank/) — diari di sessione
- Tag legacy: `git checkout legacy/avalonia-final` per recuperare lo stato
  `.NET/Avalonia` pre-pivot.
