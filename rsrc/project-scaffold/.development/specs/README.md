# Feature Specifications

This folder contains individual specifications for all {PROJECT_NAME} features.

## Structure

```text
specs/
├── implemented/     # Features already working in production
├── in-progress/     # Features currently being developed
├── planned/         # Features confirmed for upcoming releases
├── backlog/         # Validated ideas not yet scheduled
└── archived/        # Deprecated or cancelled specs
```

These five directory names are not arbitrary: `spec-workflow.py` declares them
as `STATUS_DIRS` and moves files between them by name. Renaming or dropping one
breaks the automation silently.

## Spec File Format

Each spec follows this template:

```markdown
# Feature Name

**Status**: implemented | in-progress | planned | backlog | archived
**Release**: v0.X.0 (or "unassigned")
**Priority**: must-have | should-have | nice-to-have
**Depends on**: [list of spec files]

## Summary
One sentence: what it does and why it matters.

## User Stories
- As a [user type], I want [action] so that [benefit]

## Requirements

### Functional
- [ ] Requirement 1

### Non-Functional
- Performance: ...
- Security: ...

## Technical Notes
Implementation decisions, dependencies, links to ADRs.

## Acceptance Criteria
- [ ] Verifiable criterion 1

## Open Questions
- Question that needs resolution before implementation
```

## Workflow

### Adding a New Feature

1. Create spec in `backlog/` with basic info
2. Discuss and refine requirements
3. Assign to release → move to `planned/`
4. Start development → move to `in-progress/`
5. Complete → move to `implemented/`

### Automated Lifecycle

The shipped hooks automate those moves based on git activity:

- **`post-checkout`**: on `git checkout -b {feature,fix,docs,refactor,experiment}/<name>`,
  the matching spec in `planned/<name>.md` moves to `in-progress/` and its
  frontmatter `status` is updated.
- **`post-merge`**: on no-conflict merges into `develop` — the normal case,
  since git creates those commits directly and pre-commit does not run — the
  merged branch is parsed from the merge commit subject and the matching spec
  moves to `implemented/` and is staged. The hook then prints the one-liner to
  fold it into the merge commit (`git commit --amend --no-edit`); run it right
  after the merge.
- **`pre-commit.d/05-spec-workflow`**: covers the complementary case only —
  conflicted merges concluded manually via `git commit`, where `MERGE_HEAD`
  still exists and pre-commit does run.

The script at `.development/scripts/spec-workflow.py` is the shared backend.
Missing specs, unknown branch prefixes, and non-merge commits all exit silently,
so the automation never blocks a commit. Spec filenames may carry the branch
prefix folded in (`feature/x` matches `feature-x.md`).

Activation requires `bash .development/automation/bootstrap.sh` once per clone,
which sets `core.hooksPath .githooks`.

## Naming Convention

- Use kebab-case: `column-filtering.md`, `template-validation.md`
- Be descriptive but concise
- Include the main noun: `claude-context-panel.md`, not just `claude.md`

## Cross-References

- Link to ADRs: `../reference/decisions/NNN-name.md`
- Link to tech-debt: `../tech-debt/issue-name.md`
- Link to other specs: `../planned/other-feature.md`
