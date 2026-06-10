---
type: bug
priority: medium
status: open
discovered: 2026-06-10
related: [spec-workflow-hook-silent-fail]
related_decision: null
report: null
---

# Hook di progetto (.githooks/) mai attivi: hooksPath globale li bypassa

## Problem

`.claude/rules/workflow.md` dichiara che l'attivazione degli hook di
progetto richiede `git config core.hooksPath .githooks` (una volta per
clone). Su questo clone quella config locale non è mai stata fatta:
`core.hooksPath` è settato **globalmente** in `~/.gitconfig` a
`/data/repos/.git-hooks` (hook condivisi del workspace).

Conseguenze:

- `.githooks/pre-commit.d/00-branch-protection` **non gira** — la
  protezione di `main`/`develop` documentata in workflow.md non è
  effettiva (la dir workspace non ha un modulo equivalente).
- `.githooks/pre-commit.d/02-clang-format` non gira (oggi dormiente
  comunque, manca `.clang-format`).
- Lo spec-workflow e altri moduli funzionano solo perché la dir
  workspace contiene copie quasi identiche (`06-spec-workflow`,
  `04-generate-architecture`, ...) — duplicazione che può divergere
  silenziosamente. La dir workspace ha ancora `02-dotnet-format`
  pre-pivot.

## Possible Solutions

- **Option A**: settare `core.hooksPath .githooks` localmente sul clone
  (come da doc). Attiva gli hook di progetto ma disattiva quelli
  workspace (`05-generate-readme-status`, `07-generate-index`) — da
  verificare cosa si perde.
- **Option B**: hook di progetto come orchestratore che delega anche
  alla dir workspace (chain). Più complesso, elimina la duplicazione.
- **Option C**: accettare gli hook workspace come unica fonte e
  rimuovere `.githooks/` dal progetto, portando `00-branch-protection`
  nella dir workspace. Contraddice però la filosofia "progetto
  self-contained" del workspace system.

## Recommended Approach

Decidere con Valentina: tocca la config globale del workspace, non solo
questo repo. Nel frattempo la branch protection è solo convenzionale —
attenzione ai commit diretti su `main`/`develop`.

## Related Documentation

- **Code Locations**: `.githooks/`, `/data/repos/.git-hooks/`,
  `~/.gitconfig` (`core.hooksPath`)
- `.claude/rules/workflow.md` — sezioni "Git Workflow" e "Spec
  lifecycle automation"
