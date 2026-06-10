---
type: bug
priority: low
status: closed
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

## Root Cause (2026-06-10)

Due bug indipendenti nel backend, più una scoperta di configurazione:

1. **Mismatch di naming branch → spec** (causa del silenzio). L'hook
   estrae il nome spec strippando il prefisso del branch
   (`feature/snapshot-history` → `snapshot-history`), ma i file spec
   incorporano il prefisso col trattino (`feature-snapshot-history.md`).
   `find_spec()` cercava `snapshot-history.md`, non lo trovava, e
   l'hook usciva con "No spec to update".
2. **Regex dello status sbagliata**. `move_spec()` aggiornava un campo
   markdown `**Status**:`, ma le spec usano frontmatter YAML
   (`status:`). Anche quando lo spostamento avveniva (manuale via
   script), lo status interno restava stale.
3. **Scoperta collaterale**: su questo clone `core.hooksPath` è settato
   globalmente in `~/.gitconfig` a `/data/repos/.git-hooks` (hook
   workspace). Gli hook di progetto in `.githooks/` — incluso
   `00-branch-protection` — **non girano affatto**. Lo spec-workflow
   funziona comunque perché la dir workspace contiene un
   `06-spec-workflow` identico. Tracciato separatamente, vedi Notes.

## Resolution

Fix in `.development/scripts/spec-workflow.py` (2026-06-10):

- `find_spec()`: fallback sui nomi prefissati (`<prefix>-<name>.md` per
  ogni prefisso di branch noto), accettato solo se il match è univoco.
- `move_spec()`: aggiorna prima il frontmatter YAML `status:`, con
  fallback al legacy markdown `**Status**:`.

Le tre spec wedge sono ora tutte in `specs/implemented/` con
frontmatter allineato.

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
