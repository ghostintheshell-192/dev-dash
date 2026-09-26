# Architecture Reference

Quick reference for navigating the DevDash codebase.
For detailed documentation, see `docs/`.

<!-- Hand-written: this block is not regenerated, unlike the Project Tree below.
     Keep it to what each layer is for — never a census of its files, never
     implementation status. See generate-architecture.sh for why. -->

## Layer Overview (`app/src/` — layered architecture, ADR-010)

| Layer | Path | Purpose |
|-------|------|---------|
| Entry point | `app/src/main.cpp` | `int main` → `dev_dash::app::App().Run()` |
| Composition root | `app/src/app/` | Owns every layer via `unique_ptr`; fixes construction order and drives the main loop. |
| Platform | `app/src/platform/` | SDL3/Vulkan/ImGui plumbing: window and surface lifetime, swapchain, per-frame resources, deferred destruction. |
| UI | `app/src/ui/` | ImGui panels ("panel as viewmodel", ADR-010), font library, markdown rendering on `imgui_md`. |
| Services | `app/src/services/` | Domain logic: document loading, effective-configuration resolution, diff/apply, snapshots, scaffolds. |
| Core | `app/src/core/` | Pure value types, header-only. |
| PoC (reference) | `poc/src/` | Original monolithic `Renderer` class — kept as reference pre-refactor. |

For what each directory actually contains, see the Project Tree below.

## Key Decisions

- [ADR-001: Stack tecnologico - C# + Avalonia](reference/decisions/001-stack-tecnologico.md) `[low]` — Sceglie C# + Avalonia (nativo cross-platform, MVVM) come stack definitivo per DevDash, in luogo del prototipo React e delle alternative Electron/Tauri.
- [ADR-002: Symlink vs Copy per aggregazione vault](reference/decisions/002-symlink-vs-copy.md) `[medium]` — Aggrega la documentazione nel Vault@Claude tramite symlink ai file originali, anziché copia fisica o aggregazione virtuale, per avere single source of truth e zero sync.
- [ADR-003: Issue tracking locale vs GitHub Issues](reference/decisions/003-issue-tracking-locale.md) `[medium]` — Adotta un issue tracking locale in file markdown sotto `.personal/issues/` per le issue personali, riservando GitHub Issues alle issue pubbliche e ai bug report esterni.
- [ADR-006: DevDash Desktop vs VS Code Extension](reference/decisions/006-desktop-vs-vscode-extension.md) `[high]` — Tratta DevDash Desktop ed eventuale estensione VS Code come due prodotti separati con filosofie distinte (desktop standalone vs companion dell'estensione Claude Code), integrati via filesystem e da sviluppare prima Desktop poi Extension; un addendum 2026-06-10 rimanda la ri-decisione sulla forma di distribuzione al verificarsi di un trigger concreto.
- [ADR-007: Rimozione del Terminale Embedded](reference/decisions/007-rimozione-terminale-embedded.md) `[medium]` — Rimuove completamente la feature del terminale embedded (PTY, ANSI parser, dipendenza Pty.Net) senza sostituirla, riallineando DevDash alla filosofia "documentazione e contesto, non esecuzione" e delegando l'esecuzione di Claude Code al terminale esterno.
- [ADR-008: Pivot dello stack — da C#/Avalonia a C++/Dear ImGui](reference/decisions/008-pivot-to-cpp-imgui.md) `[high]` — Riscrive DevDash in C++20 con Dear ImGui (docking) su SDL3 + Vulkan, abbandonando .NET 8 + Avalonia, motivato da preferenza linguistica, rifiuto degli user agreement Avalonia e dal kickstart Germen Pulchrum; il legacy resta sotto il tag `legacy/avalonia-final` e il nuovo codice vive in `poc/`.
- [ADR-009: Libreria markdown — imgui_md + MD4C](reference/decisions/009-markdown-library-imgui-md.md) `[medium]` — Adotta `imgui_md` (mekhontsev) + MD4C come stack markdown, separando il parser CommonMark esterno e mantenuto (MD4C) dal bridge ImGui disposable, invece del parser hand-written di `imgui_markdown` upstream o del fork mgerhardy; DevDash deriva `MarkdownRenderer` dal bridge per integrarvi font e color scheme.
- [ADR-010: Architettura del progetto vero — split layered](reference/decisions/010-architecture-design.md) `[critical]` — Struttura il progetto vero come split layered a cinque cartelle (`core/services/ui/platform/app/`, promozione di `poc/` → `app/`), con pattern "panel as viewmodel" invece di MVVM, classi concrete invece di interfacce virtuali, e callback con ordine di dichiarazione per il `MarkdownRenderer`.
- [ADR-011: Modello di release e distribuzione](reference/decisions/011-release-and-distribution.md) `[medium]` — Definisce il modello di release Linux-first minimo ma scalabile: SemVer con tag `v*` su `main` e versione nel `CMakeLists.txt`, install rules CMake con `GNUInstallDirs`, risoluzione asset a runtime a due tentativi con DejaVu bundled, e packaging via CPack (generator `TGZ` come primo formato).
- [ADR-012: Automazione a due livelli, agnostica rispetto al codebase](reference/decisions/012-codebase-agnostic-automation.md) `[high]` — Riduce l'automazione a due livelli (globale Claude + progetto self-contained, decommissionando il livello workspace) e separa orchestrazione e implementazione: gli hook diventano orchestratori generici e agnostici rispetto allo stack, mentre la conoscenza stack-specifica vive in entry point standard sotto `.development/automation/`.
- [ADR-013: Scaffold source of truth — rsrc versionato, symlink in dev](reference/decisions/013-scaffold-source-of-truth.md) `[critical]` — Stabilisce `rsrc/project-scaffold/` come unica fonte di verità versionata in git, con `~/.devdash/scaffolds/dev-dash-standard` come symlink ad essa in modalità dev (il promote dell'app scrive nel working tree) e come copia per gli utenti finali, eliminando ogni meccanismo di sync.
- [ADR-014: Strategia di co-evoluzione con Germen Pulchrum](reference/decisions/014-germen-coevolution-strategy.md) `[high]` — Articola la relazione con Germen Pulchrum come co-evoluzione su uno spettro temporale (copia con attribuzione oggi → `git subtree` poi → adozione wholesale tendenziale), abilita la contribuzione bidirezionale attiva (Valentina collaboratrice), e fissa che il modulo grafi nasca su Germen co-sviluppato ma vincolato a essere portabile (solo Dear ImGui, layout/render disaccoppiati); un addendum 2026-09-26 registra che il modulo è nato come libreria autonoma di Dario (ImGuiDot, layout Graphviz) consumata via CPM.
- [ADR-015: Scaffold templating — sostituzione esplicita guidata da manifest](reference/decisions/015-scaffold-templating.md) `[high]` — Rifonda `apply`/`promote` dello scaffold come trasformazioni inverse fra forma *template* (placeholder) e forma *concreta*: la risoluzione dei parametri è esplicita e guidata da un manifest dichiarato (editato solo da GUI, residui `required` bloccanti), superando l'inclinazione "no-substitution" di `feature-scaffold-management`.
- [ADR-016: Versionamento git gestito degli scaffold](reference/decisions/016-git-managed-scaffold-versioning.md) `[high]` — DevDash versiona gli scaffold con git in modo gestito — un repo per scaffold, commit "sotto il cofano" con diff mostrati, git come unica storia degli scaffold — astenendosi sui symlink/repo esterni (dogfooding di ADR-013); apre all'integrazione progressiva di git nella codebase (diff-compute, merge).
- [ADR-017: Estrazione del code graph — tree-sitter di base, libclang opzionale](reference/decisions/017-code-graph-extraction.md) `[medium]` — Il code graph estrae la struttura del codice con tree-sitter più un risolutore di nomi nostro, sempre disponibile e dichiarato con i suoi limiti; libclang (C, C++, Objective-C) è un componente opzionale caricato a runtime, che l'utente scarica solo se vuole l'analisi avanzata; per il C# l'equivalente sarebbe Roslyn.

## Project Tree

> Auto-generated from source code.
> Run `.development/scripts/generate-architecture.sh` to update.


### app/src
- `main.cpp`
- `pch.h`

### app/src/app
- `app.cpp`
- `app.h`

### app/src/core
- `config_layer.h`
- `diff_entry.h`
- `effective_config.h`
- `line_diff.h`
- `project.h`
- `scaffold.h`
- `snapshot.h`

### app/src/platform
- `asset_paths.cpp`
- `asset_paths.h`
- `deletion_queue.h` — Adapted from Germen Pulchrum (DPD85/Germen, MIT) — `CodaCancellazione`. See app/THIRD_PARTY_NOTICES.md for attribution.
- `frame_resources.cpp`
- `frame_resources.h`
- `imgui_backend.cpp`
- `imgui_backend.h`
- `sdl_session.cpp`
- `sdl_session.h`
- `swapchain.cpp`
- `swapchain.h`
- `vulkan_context.cpp`
- `vulkan_context.h`
- `window.cpp`
- `window.h`

### app/src/services
- `adapter_utils.h`
- `agents_adapter.cpp`
- `agents_adapter.h`
- `apply_engine.cpp`
- `apply_engine.h`
- `claude_md_adapter.cpp`
- `claude_md_adapter.h`
- `config_file_scanner.cpp`
- `config_file_scanner.h`
- `config_resolver.cpp`
- `config_resolver.h`
- `diff_engine.cpp`
- `diff_engine.h`
- `document_loader.cpp`
- `document_loader.h`
- `hooks_adapter.cpp`
- `hooks_adapter.h`
- `mcp_adapter.cpp`
- `mcp_adapter.h`
- `memory_adapter.cpp`
- `memory_adapter.h`
- `promote_engine.cpp`
- `promote_engine.h`
- `rules_adapter.cpp`
- `rules_adapter.h`
- `scaffold_repository.cpp`
- `scaffold_repository.h`
- `settings_parser.cpp`
- `settings_parser.h`
- `skills_adapter.cpp`
- `skills_adapter.h`
- `snapshot_service.cpp`
- `snapshot_service.h`

### app/src/ui
- `document_panel_host.cpp`
- `document_panel_host.h`
- `effective_config_panel.cpp`
- `effective_config_panel.h`
- `file_diff_panel.cpp`
- `file_diff_panel.h`
- `font_library.cpp`
- `font_library.h`
- `markdown_renderer.cpp`
- `markdown_renderer.h`
- `project_selector_panel.cpp`
- `project_selector_panel.h`
- `scaffold_diff_panel.cpp`
- `scaffold_diff_panel.h`
- `shell.cpp`
- `shell.h`
- `sidebar.cpp`
- `sidebar.h`
- `snapshot_history_panel.cpp`
- `snapshot_history_panel.h`
- `status_sink.h`
- `theme.cpp`
- `theme.h`
- `widgets.cpp`
- `widgets.h`

---

*Auto-generated by `.development/scripts/generate-architecture.sh`*
