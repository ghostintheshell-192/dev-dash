# Git Workflow

Branch management, commit standards, and git best practices.

---

## Branch Strategy - MANDATORY

**NEVER work directly on main branch**

**⚠️ CRITICAL: ALWAYS use `develop` as default branch, even if user says "main" by mistake**

### Branch Naming Conventions

| Prefix | Use Case |
|--------|----------|
| `feature/description` | New functionality |
| `fix/description` | Bug fixes |
| `refactor/description` | Code improvements |
| `docs/description` | Documentation updates |
| `chore/description` | Maintenance tasks |

### Create feature branch for each task

```bash
git checkout -b feature/task-description
# Example: git checkout -b feature/streamlit-dashboard
# Example: git checkout -b fix/button-lifecycle-bug
# Example: git checkout -b refactor/settings-architecture
```

---

## Workflow Process

```bash
# Start new work (DEFAULT: use develop)
git checkout develop
git pull origin develop
git checkout -b feature/my-new-feature

# Work and commit
git add .
git commit -m "descriptive message"

# Push feature branch
git push -u origin feature/my-new-feature

# Merge when ready
git checkout develop
git merge feature/my-new-feature
git push origin develop

# Cleanup
git branch -d feature/my-new-feature
git push origin --delete feature/my-new-feature
```

---

## When to Create Branches

| Situation | Create Branch? |
|-----------|----------------|
| Working on roadmap items | ✅ Always |
| Implementing new features | ✅ Always |
| Fixing bugs | ✅ Always |
| Refactoring code | ✅ Always |
| Initial project setup | ❌ Exception (can use main) |

---

## Branch Lifecycle

1. Create branch from latest `develop` (NOT main)
2. Work on single focused task
3. Commit frequently with clear messages
4. Test thoroughly before merge
5. Clean up branch after merge

---

## Commit Message Standards

- Use descriptive commit messages
- Include context and reasoning
- Follow conventional commits format when possible
- Always include Claude Code attribution

### Conventional Commits Format

```
type(scope): description

[optional body]

[optional footer]
```

Examples:
- `feat(search): add fuzzy matching support`
- `fix(export): handle empty cells correctly`
- `refactor(core): extract validation logic`
- `docs(readme): update installation instructions`

---

## Pre-Flight Checks

Before starting work, verify:

1. **Repository clean**: `git status` shows no uncommitted changes
2. **Correct branch**: NOT on `main` — create feature branch if needed
3. **Up to date**: `git pull` from remote

See also: `.rules/preflight-checks.md` for detailed checklist.

---

## Merge Strategy

- Use **merge** for integrating branches (NOT rebase)
- `git merge feature/branch` preserves history
- **NEVER USE REBASE** for shared branches

---

*For destructive git operations (force push, reset), see `core/security-boundaries.md`*
