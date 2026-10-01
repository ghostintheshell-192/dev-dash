# INDEX - DevDash Development Documentation

*A map of what exists and what each document is for.*
*For when and why something changed, ask git.*

---

## Quick Links

- [Current Status](CURRENT-STATUS.md)
- [Tech Debt](tech-debt/README.md)
- [Specs](specs/README.md)
- [ADR](reference/decisions/README.md)

---

## Development Documentation (.development/)

*Specs, tech-debt, decisions*

### (root)/ (5 files)

- [ARCHITECTURE.md](ARCHITECTURE.md) — Architecture Reference
- [CURRENT-STATUS.md](CURRENT-STATUS.md) — DevDash - Current Status
- [INDEX.md](INDEX.md) — INDEX - DevDash Development Documentation
- [README.md](README.md) — .development/ - Development Documentation
- [api-design.md](api-design.md) — API Design — Layered Architecture

### specs/ (1 files)

- [README.md](specs/README.md) — Feature Specifications

### specs/archived/ (5 files)

- [feature-claude-context.md](specs/archived/feature-claude-context.md) — Feature: Claude Context Panel
- [feature-embedded-terminal.md](specs/archived/feature-embedded-terminal.md) — Embedded Claude Terminal
- [feature-issue-management.md](specs/archived/feature-issue-management.md) — Issue Management UI
- [feature-markdown-rendering.md](specs/archived/feature-markdown-rendering.md) — Markdown Rendering & Editing
- [roadmap.md](specs/archived/roadmap.md) — Roadmap DevDash

### specs/backlog/ (3 files)

- [feature-global-settings-coverage.md](specs/backlog/feature-global-settings-coverage.md) — Global Settings Coverage (parsing generalizzato dei JSON di .claude)
- [feature-plugins-keybindings-coverage.md](specs/backlog/feature-plugins-keybindings-coverage.md) — Plugins & Keybindings Coverage
- [feature-runtime-view-of-truth.md](specs/backlog/feature-runtime-view-of-truth.md) — Runtime View-of-Truth

### specs/implemented/ (6 files)

- [feature-agnostic-automation.md](specs/implemented/feature-agnostic-automation.md) — Agnostic Automation (two-level)
- [feature-effective-config-view.md](specs/implemented/feature-effective-config-view.md) — Effective Config View
- [feature-release-readiness.md](specs/implemented/feature-release-readiness.md) — Release Readiness
- [feature-scaffold-management.md](specs/implemented/feature-scaffold-management.md) — Scaffold Management
- [feature-snapshot-history.md](specs/implemented/feature-snapshot-history.md) — Snapshot & History
- [feature-ui-overhaul.md](specs/implemented/feature-ui-overhaul.md) — feature-ui-overhaul

### specs/in-progress/ (1 files)

- [feature-code-graph.md](specs/in-progress/feature-code-graph.md) — Code Graph — class diagram on demand

### specs/planned/ (1 files)

- [feature-scaffold-templating.md](specs/planned/feature-scaffold-templating.md) — Scaffold Templating

### tech-debt/ (15 files)

- [README.md](tech-debt/README.md) — Tech Debt Issues
- [_TEMPLATE.md](tech-debt/_TEMPLATE.md) — [Issue Title]
- [docs-update-orchestrator-hardcodes-generated-files.md](tech-debt/docs-update-orchestrator-hardcodes-generated-files.md) — `04-docs-update` declares it knows nothing about the generators, then hardcodes their outputs
- [imguidot-box-size-font-metrics.md](tech-debt/imguidot-box-size-font-metrics.md) — ImGuiDot: node boxes far larger than their text
- [imguidot-diagram-size-api.md](tech-debt/imguidot-diagram-size-api.md) — ImGuiDot: no API for the size of a diagram
- [imguidot-fillcolor-without-filled.md](tech-debt/imguidot-fillcolor-without-filled.md) — ImGuiDot: `fillcolor` applied without `style=filled`
- [imguidot-line-styles-ignored.md](tech-debt/imguidot-line-styles-ignored.md) — ImGuiDot: `style=dashed` and `style=dotted` ignored
- [imguidot-record-shape.md](tech-debt/imguidot-record-shape.md) — ImGuiDot: `shape=record` not drawn
- [imguidot-reserved-space-border.md](tech-debt/imguidot-reserved-space-border.md) — ImGuiDot: reserved space cuts the outer borders
- [line-level-promote.md](tech-debt/line-level-promote.md) — Promote e Apply operano solo sull'intero file
- [markdown-code-block-styling.md](tech-debt/markdown-code-block-styling.md) — Fenced code blocks rendered as flat yellow text — no syntax highlighting
- [non-markdown-files-rendered-as-markdown.md](tech-debt/non-markdown-files-rendered-as-markdown.md) — File non-Markdown renderizzati come Markdown (script, config)
- [preprocess-imports-indented-fences.md](tech-debt/preprocess-imports-indented-fences.md) — PreprocessImports does not recognise indented fenced code blocks
- [scanner-directory-include-silent.md](tech-debt/scanner-directory-include-silent.md) — ConfigFileScanner: @include verso directory accettato e poi fallisce in silenzio
- [tree-sitter-cpp-default-argument-braces.md](tech-debt/tree-sitter-cpp-default-argument-braces.md) — tree-sitter-cpp: `= {}` default argument parsed as an error

### reference/decisions/ (17 files)

- [001-stack-tecnologico.md](reference/decisions/001-stack-tecnologico.md) — ADR-001: Stack tecnologico - C# + Avalonia
- [002-symlink-vs-copy.md](reference/decisions/002-symlink-vs-copy.md) — ADR-002: Symlink vs Copy per aggregazione vault
- [003-issue-tracking-locale.md](reference/decisions/003-issue-tracking-locale.md) — ADR-003: Issue tracking locale vs GitHub Issues
- [006-desktop-vs-vscode-extension.md](reference/decisions/006-desktop-vs-vscode-extension.md) — ADR-006: DevDash Desktop vs VS Code Extension
- [007-rimozione-terminale-embedded.md](reference/decisions/007-rimozione-terminale-embedded.md) — ADR-007: Rimozione del Terminale Embedded
- [008-pivot-to-cpp-imgui.md](reference/decisions/008-pivot-to-cpp-imgui.md) — ADR-008: Pivot dello stack — da C#/Avalonia a C++/Dear ImGui
- [009-markdown-library-imgui-md.md](reference/decisions/009-markdown-library-imgui-md.md) — ADR-009: Libreria markdown — imgui_md + MD4C
- [010-architecture-design.md](reference/decisions/010-architecture-design.md) — ADR-010: Architettura del progetto vero — split layered
- [011-release-and-distribution.md](reference/decisions/011-release-and-distribution.md) — ADR-011: Modello di release e distribuzione
- [012-codebase-agnostic-automation.md](reference/decisions/012-codebase-agnostic-automation.md) — ADR-012: Automazione a due livelli, agnostica rispetto al codebase
- [013-scaffold-source-of-truth.md](reference/decisions/013-scaffold-source-of-truth.md) — ADR-013: Scaffold source of truth — rsrc versionato, symlink in dev
- [014-germen-coevolution-strategy.md](reference/decisions/014-germen-coevolution-strategy.md) — ADR-014: Strategia di co-evoluzione con Germen Pulchrum
- [015-scaffold-templating.md](reference/decisions/015-scaffold-templating.md) — ADR-015: Scaffold templating — sostituzione esplicita guidata da manifest
- [016-git-managed-scaffold-versioning.md](reference/decisions/016-git-managed-scaffold-versioning.md) — ADR-016: Versionamento git gestito degli scaffold
- [017-code-graph-extraction.md](reference/decisions/017-code-graph-extraction.md) — ADR-017: Estrazione del code graph — tree-sitter di base, libclang opzionale
- [README.md](reference/decisions/README.md) — Architecture Decision Records
- [_TEMPLATE.md](reference/decisions/_TEMPLATE.md) — ADR-NNN: <titolo conciso della decisione>

### reference/technical/ (1 files)

- [resource-model.md](reference/technical/resource-model.md) — DevDash — Resource Model (DRAFT)

### reference/technical/code-graph-extraction/ (1 files)

- [README.md](reference/technical/code-graph-extraction/README.md) — Code-graph extraction experiment (2026-09-26)

### archive/analysis/ (1 files)

- [2025-12-11_report_code-reviewer.md](archive/analysis/2025-12-11_report_code-reviewer.md) — Code Review Report - DevDash

### archive/completed/ (15 files)

- [2026-05-10_contentcontrol-binding-multisidebar.md](archive/completed/2026-05-10_contentcontrol-binding-multisidebar.md) — ContentControl Binding Error in MultiSidebar - Empty Sidebar Content
- [2026-05-10_imgui-backend-shutdown-order.md](archive/completed/2026-05-10_imgui-backend-shutdown-order.md) — ImGui assertion on exit: backend non spento prima di DestroyContext
- [2026-06-10_project-githooks-not-active.md](archive/completed/2026-06-10_project-githooks-not-active.md) — Hook di progetto (.githooks/) mai attivi: hooksPath globale li bypassa
- [2026-06-11_spec-workflow-hook-silent-fail.md](archive/completed/2026-06-11_spec-workflow-hook-silent-fail.md) — Hook 05 spec-workflow non sposta le spec al merge
- [2026-06-28_scaffold-architecture-scripts.md](archive/completed/2026-06-28_scaffold-architecture-scripts.md) — Include architecture scripts in project scaffold
- [2026-07-25_archive-hook-loses-the-deletion.md](archive/completed/2026-07-25_archive-hook-loses-the-deletion.md) — 03-archive-resolved-issues commits the archived copy but never the removal of the original
- [2026-07-25_handoff-command-stale-projects-nesting.md](archive/completed/2026-07-25_handoff-command-stale-projects-nesting.md) — `/handoff` command contradicts the session-handoff skill on where handoffs live
- [2026-07-25_scaffold-architecture-eval-glob-expansion.md](archive/completed/2026-07-25_scaffold-architecture-eval-glob-expansion.md) — generate-architecture.sh: eval re-expands unquoted globs against the CWD
- [2026-07-25_scaffold-doc-generators-not-idempotent.md](archive/completed/2026-07-25_scaffold-doc-generators-not-idempotent.md) — Doc generators rewrite their output on every run (INDEX.md, tech-debt/README.md)
- [2026-07-25_tech-debt-index-backslash-escape.md](archive/completed/2026-07-25_tech-debt-index-backslash-escape.md) — update-tech-debt-index.py crashes on backslashes in issue titles
- [2026-08-29_architecture-layer-overview-stale-prose.md](archive/completed/2026-08-29_architecture-layer-overview-stale-prose.md) — ARCHITECTURE.md "Layer Overview" table is hand-written prose frozen at the ADR-010 pivot, not derived from source
- [2026-08-29_claude-md-rules-list-incomplete.md](archive/completed/2026-08-29_claude-md-rules-list-incomplete.md) — .claude/CLAUDE.md's rule enumeration omits idea-capture.md
- [2026-08-29_post-merge-does-not-regenerate-derived-docs.md](archive/completed/2026-08-29_post-merge-does-not-regenerate-derived-docs.md) — Derived docs are one regeneration behind after any merge of two independent branches
- [2026-08-29_tech-debt-index-generator-cannot-bootstrap.md](archive/completed/2026-08-29_tech-debt-index-generator-cannot-bootstrap.md) — `update-tech-debt-index.py` cannot create the README it maintains, unlike its two siblings
- [2026-09-30_document-links-not-followed.md](archive/completed/2026-09-30_document-links-not-followed.md) — I link nei documenti non si aprono

---

## Public Documentation (docs/)

*Committed to git - user-facing documentation*

### docs/

- [ARCHITECTURE.md](../docs/ARCHITECTURE.md) — DevDash Architecture
- [README.md](../docs/README.md) — Documentation
- [SETUP.md](../docs/SETUP.md) — Setup DevDash

---

*Run `python .development/scripts/generate-index.py` to regenerate*