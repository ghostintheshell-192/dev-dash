---
type: bug
priority: low
status: open
discovered: 2026-05-10
related: []
related_decision: null
report: null
---

# Hook 05 spec-workflow non sposta le spec al merge

## Problem

Il pre-commit hook `.githooks/pre-commit.d/05-spec-workflow` dovrebbe
spostare la spec da `specs/planned/` a `specs/implemented/` automaticamente
quando si fa un merge commit in `develop` da un branch `feature/<name>`.

Al merge di `feature/effective-config-view`, l'hook non ha mosso
`specs/planned/feature-effective-config-view.md` né aggiornato il frontmatter.
La spec è stata spostata manualmente.

## Analysis

L'hook condivide il backend `.development/scripts/spec-workflow.py`.
Non è stato indagato il motivo esatto del silenzio — lo script è progettato
per uscire silenziosamente su casi non riconosciuti (merge non-detect, spec
mancante, branch prefix non noto).

Possibili cause:
- Il git message del merge commit non è nella forma attesa dallo script
- Il rilevamento "è un merge commit?" fallisce in questo contesto hook
- Il path lookup della spec differisce da quello aspettato

## Possible Solutions

- **Option A**: Aggiungere logging temporaneo a `spec-workflow.py` e
  riprodurre il merge in ambiente controllato per vedere l'output.
- **Option B**: Riscrivere il rilevamento merge con `git rev-parse --verify MERGE_HEAD`
  invece di analizzare il messaggio di commit.
- **Option C**: Eseguire `spec-workflow.py` come post-merge hook invece
  che come pre-commit, dove l'informazione sul merge è più accessibile.

## Recommended Approach

Option B + C da valutare insieme: cambiare il punto di innesco (post-merge
hook) e irrobustire il rilevamento. Non urgente finché la wedge è piccola
e il workaround manuale è veloce.

## Notes

Workaround corrente: spostare la spec manualmente con `git mv` su un branch
`docs/` e aggiornare il frontmatter `status`.

## Related Documentation

- **Code Locations**: `.githooks/pre-commit.d/05-spec-workflow`,
  `.development/scripts/spec-workflow.py`
- `.claude/rules/workflow.md` — sezione "Spec lifecycle automation"
