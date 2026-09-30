# Feature Specifications

This folder contains individual specifications for all DevDash features.

## Structure

```text
specs/
├── implemented/     # Features already working in production
├── in-progress/     # Features currently being developed
├── planned/         # Features confirmed for upcoming releases
├── backlog/         # Validated ideas not yet scheduled
└── archived/        # Deprecated or cancelled specs
```

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
- [ ] Requirement 2

### Non-Functional
- Performance: ...
- Security: ...

## Technical Notes
Implementation decisions, dependencies, links to ADRs.

## Acceptance Criteria
- [ ] Verifiable criterion 1
- [ ] Verifiable criterion 2

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

The repo ships hooks that automate spec moves based on git activity:

- **`post-checkout`**: on `git checkout -b feature/<name>`, the matching spec
  in `planned/<name>.md` is moved to `in-progress/`.
- **`pre-commit.d/05-spec-workflow`**: on merge commits into `develop`, the
  matching spec is moved to `implemented/`.

The script at `.development/scripts/spec-workflow.py` is the shared backend.
Activation requires `git config core.hooksPath .githooks` (done once per clone).

## Naming Convention

- Use kebab-case: `column-filtering.md`, `template-validation.md`
- Be descriptive but concise
- Include the main noun: `claude-context-panel.md` not just `claude.md`

## Cross-References

- Link to ADRs: `../reference/decisions/NNN-name.md`
- Link to tech-debt: `../tech-debt/issue-name.md`
- Link to other specs: `../planned/other-feature.md`
