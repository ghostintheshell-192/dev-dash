# Git hooks

These are the project-level hooks. `pre-commit` runs the modules in
`pre-commit.d/` in numeric order.

**They stay inactive until you enable them**, once per clone:

```bash
bash .development/automation/bootstrap.sh
```

Git never activates hooks that ship inside a repository. That is deliberate:
it is a security property. `bootstrap.sh` takes care of three things:

- it sets `core.hooksPath .githooks` locally, which overrides any global
  hooksPath;
- it registers the `merge=generated` driver used by the derived docs in
  `.gitattributes`;
- it makes the hooks and entry points executable.

Hooks and CI only orchestrate: they contain no stack-specific commands
(ADR-012). All knowledge of the stack lives in the entry points under
`.development/automation/`.

## pre-commit modules

| Module | What it does | Blocks? |
| ------ | ------------ | ------- |
| `00-branch-protection` | On `main` and `develop` it allows only merge commits, plus the spec-bookkeeping amend of a merge that has not been pushed yet. Everything else needs a task branch. | yes |
| `01-security` | Scans staged files for secrets, by filename, by content pattern and by directory. | yes |
| `02-format-check` | Runs `.development/automation/format-check.sh` on the staged files. It does nothing when that entry point is missing. | yes |
| `03-archive-resolved-issues` | Moves resolved, closed and rejected tech-debt issues to `.development/archive/completed/`. | no |
| `04-docs-update` | Runs `.development/automation/docs-update.sh` and stages the regenerated docs. | no |
| `05-spec-workflow` | When a merge is concluded by hand with `git commit` (a conflicted merge), it moves the merged branch's spec to `specs/implemented/`. | no |

If a check is wrong, bypass it with `git commit --no-verify`. That escape
hatch is the price of a false positive.

## post-checkout

`post-checkout` does two things:

- It warns when you check out a protected branch.
- On `git checkout -b <prefix>/<name>` it moves the matching spec from
  `specs/planned/` to `specs/in-progress/`.

## post-merge

`post-merge` runs after every merge, including the one `git pull` makes.

Git creates a merge commit without conflicts **without running pre-commit**.
So this hook does the work pre-commit would otherwise miss:

- It moves the merged branch's spec to `specs/implemented/`. It reads the
  branch name from the subject of the merge commit.
- It regenerates the derived docs from the merged tree.

It stages what it changes but does not commit. It prints the one-liner that
folds the changes into the merge commit (`git commit --amend --no-edit`).

## Relationship to the remote gate

These hooks are only the *local* half of branch protection. If GitHub branch
protection is configured for the repository, that is the remote half. The two
complement each other: the local hooks keep day-to-day commits off `main` and
`develop`, and the remote settings remain the escape hatch when there is a
genuine emergency.
