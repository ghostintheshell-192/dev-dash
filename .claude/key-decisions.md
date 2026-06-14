# Key Decisions

⚠️ **Auto-generated digest of high-impact architecture decisions (ADR Impact ≥ high).**
Loaded into every session via the @include in `.claude/CLAUDE.md`. Open the linked ADR for full context.

## Critical — must not be violated

Constraints that apply across the whole codebase.

- **[ADR-010: Architettura del progetto vero — split layered](../.development/reference/decisions/010-architecture-design.md)** — Struttura il progetto vero come split layered a cinque cartelle (`core/services/ui/platform/app/`, promozione di `poc/` → `app/`), con pattern "panel as viewmodel" invece di MVVM, classi concrete invece di interfacce virtuali, e callback con ordine di dichiarazione per il `MarkdownRenderer`.
- **[ADR-013: Scaffold source of truth — rsrc versionato, symlink in dev](../.development/reference/decisions/013-scaffold-source-of-truth.md)** — Stabilisce `rsrc/project-scaffold/` come unica fonte di verità versionata in git, con `~/.devdash/scaffolds/dev-dash-standard` come symlink ad essa in modalità dev (il promote dell'app scrive nel working tree) e come copia per gli utenti finali, eliminando ogni meccanismo di sync.

## High-impact context

Decisions that shape ongoing work — know these before deciding.

- **[ADR-006: DevDash Desktop vs VS Code Extension](../.development/reference/decisions/006-desktop-vs-vscode-extension.md)** — Tratta DevDash Desktop ed eventuale estensione VS Code come due prodotti separati con filosofie distinte (desktop standalone vs companion dell'estensione Claude Code), integrati via filesystem e da sviluppare prima Desktop poi Extension; un addendum 2026-06-10 rimanda la ri-decisione sulla forma di distribuzione al verificarsi di un trigger concreto.
- **[ADR-008: Pivot dello stack — da C#/Avalonia a C++/Dear ImGui](../.development/reference/decisions/008-pivot-to-cpp-imgui.md)** — Riscrive DevDash in C++20 con Dear ImGui (docking) su SDL3 + Vulkan, abbandonando .NET 8 + Avalonia, motivato da preferenza linguistica, rifiuto degli user agreement Avalonia e dal kickstart Germen Pulchrum; il legacy resta sotto il tag `legacy/avalonia-final` e il nuovo codice vive in `poc/`.
- **[ADR-012: Automazione a due livelli, agnostica rispetto al codebase](../.development/reference/decisions/012-codebase-agnostic-automation.md)** — Riduce l'automazione a due livelli (globale Claude + progetto self-contained, decommissionando il livello workspace) e separa orchestrazione e implementazione: gli hook diventano orchestratori generici e agnostici rispetto allo stack, mentre la conoscenza stack-specifica vive in entry point standard sotto `.development/automation/`.
- **[ADR-014: Strategia di co-evoluzione con Germen Pulchrum](../.development/reference/decisions/014-germen-coevolution-strategy.md)** — Articola la relazione con Germen Pulchrum come co-evoluzione su uno spettro temporale (copia con attribuzione oggi → `git subtree` poi → adozione wholesale tendenziale), abilita la contribuzione bidirezionale attiva (Valentina collaboratrice), e fissa che il modulo grafi nasca su Germen co-sviluppato ma vincolato a essere portabile (solo Dear ImGui, layout/render disaccoppiati).

---

*Auto-generated from ADRs with Impact ≥ high. Run `.development/scripts/generate-claude-config.sh` to update.*
