# Pre-flight Checks - Detailed Logic

Pre-flight checks ensure the development environment is safe and properly configured before making code modifications.

## When to Run

**Run pre-flight checks when:**
- User requests code modifications ("implement...", "add...", "fix...")
- User says "work on..." or "start working on..."
- About to make changes that will be committed

**Skip pre-flight checks when:**
- Read-only operations (exploring, reading files, explaining code)
- Already on a feature branch and continuing work
- User explicitly requests to skip checks

## Check Sequence

### 1. Git Repository Verification

```bash
git rev-parse --git-dir
```

**If not a git repository:**
- ⚠️ Warn: "This directory is not a git repository"
- 💡 Suggest: "Would you like me to initialize git? (git init)"
- ⏸️ Wait for user confirmation before proceeding

**If git repository exists:**
- ✅ Proceed to next check

---

### 2. Current Branch Check

```bash
git branch --show-current
```

**If on `main` or `master` branch:**
- 🛑 **STOP IMMEDIATELY**
- ⚠️ Critical warning: "You are on the main branch. Creating feature branch is required."
- 🔍 Check if appropriate branch exists:
  ```bash
  git branch --list "feature/*" "fix/*" "refactor/*"
  ```
- If relevant branch found: "Found existing branch: feature/ui-improvements. Switch to it?"
- If no relevant branch: "Creating new branch: feature/{task-name}"
- ✅ Create/checkout branch before proceeding

**If on `develop` branch:**
- 💡 Suggest: "Consider creating a feature branch for this task"
- If task name provided: "Create feature/{task-name}?"
- ✅ Allow user to decide (can continue on develop for small changes)

**If on feature branch:**
- ✅ Good! Continue with remaining checks

---

### 3. Existing Branch Detection

```bash
# List all feature branches
git branch --list "feature/*" "fix/*" "refactor/*" "docs/*" "chore/*"

# Check if task-related branch already exists
git branch --list "*{task-keyword}*"
```

**Logic:**
- Extract keywords from task description
- Search for existing branches matching keywords
- If found: "Found existing branch: feature/ui-improvements (related to your task). Use it?"
- If multiple found: Show list, ask user to choose
- If none found: Create new branch with appropriate name

**Generic branches (reusable):**
- `feature/ui-improvements` → Reuse for UI-related tasks
- `feature/refactoring` → Reuse for refactoring work
- `feature/bug-fixes` → Reuse for minor bug fixes

---

### 4. Uncommitted Changes Check

```bash
git status --porcelain
```

**Configuration:** Based on `user-preferences.yaml` → `preflight_checks.checks.uncommitted_changes`

**If "warn" mode (default):**
- ⚠️ Notify: "You have uncommitted changes in X files"
- 📋 List modified files (up to 5, then "... and X more")
- ❓ Ask: "Continue with current changes, commit first, or stash?"
- ✅ Respect user's choice

**If "block" mode:**
- 🛑 Stop: "Cannot proceed with uncommitted changes"
- 💡 Suggest: "Please commit or stash changes first"
- ⏸️ Wait for user action

**If "ignore" mode:**
- ✅ Proceed silently

**If no uncommitted changes:**
- ✅ Proceed to next check

---

### 5. Remote Configuration Check

```bash
git remote -v
```

**Configuration:** Based on `user-preferences.yaml` → `preflight_checks.checks.remote_configured`

**If "info" mode (default):**
- ℹ️ Info: "Remote: origin (git@github.com:user/repo.git)" or "No remote configured (local only)"
- ✅ Continue (informational only)

**If "warn" mode:**
- ⚠️ Warn if no remote: "No git remote configured. Changes will only be local."
- ✅ Continue

**If "ignore" mode:**
- ✅ Skip check entirely

---

### 6. Untracked Important Files Check

```bash
git ls-files --others --exclude-standard
```

**Configuration:** Based on `user-preferences.yaml` → `preflight_checks.checks.untracked_files`

**Important file patterns to detect:**
- `*.csproj`, `*.sln` (C# projects)
- `pubspec.yaml` (Flutter)
- `package.json` (JavaScript/TypeScript)
- `requirements.txt`, `pyproject.toml` (Python)
- `README.md`, `LICENSE`, `CLAUDE.md` (Documentation)
- `appsettings.json`, `config.json` (Configuration files)

**If important untracked files found:**
- ℹ️ Info: "Found untracked important files:"
  - `ExcelViewer.csproj`
  - `README.md`
- ❓ Ask: "Would you like to add these to git?"
- ✅ Respect user's choice

**If "ignore" mode or no important files:**
- ✅ Proceed

---

## Output Format

### Compact Format (default)

```
🔍 Pre-flight checks...
✅ Git repository
✅ Branch: feature/excel-export
⚠️  Uncommitted: 3 files (ExcelExporter.cs, README.md, appsettings.json)
✅ Remote: origin
```

### Verbose Format (if issues found)

```
🔍 Pre-flight checks...

✅ Git repository: Initialized
⚠️  Current branch: main
    → Creating feature branch: feature/excel-export

✅ Feature branch: feature/excel-export (created)
⚠️  Uncommitted changes: 3 files modified
    • ExcelExporter.cs
    • README.md
    • appsettings.json

    Continue with current changes? [Y/n]

✅ Remote: origin (git@github.com:user/excel-viewer.git)
ℹ️  Untracked files: None (or no important files)

→ Ready to proceed!
```

---

## Configuration Reference

See `.rules/user-preferences.yaml`:

```yaml
preflight_checks:
  enabled: true                     # Master switch

  checks:
    git_initialized: true            # Check git repo exists
    current_branch: true             # Enforce branch strategy
    uncommitted_changes: "warn"      # warn | block | ignore
    remote_configured: "info"        # info | warn | ignore
    untracked_files: "info"          # info | warn | ignore

  auto_create_feature_branch: true  # Auto-create if on main
  auto_branch_naming: "feature/{task}"
  check_existing_branch: true       # Look for existing relevant branches

  run_on:
    - "code_modifications"
    - "explicit_work_request"

  skip_on:
    - "read_only"
    - "already_in_feature"
```

---

## Integration with Workflow

Pre-flight checks run at **Step 12** of the Coding Standards Workflow (see `./CLAUDE.md`).

**Workflow integration:**
1. Load standards (steps 1-11)
2. **Run pre-flight checks** (step 12a)
3. Validate branch strategy (step 12b)
4. Validate task clarity (step 12c)
5. Confirm standards if verbose (step 12d)
6. Apply core principles (step 12e)
7. → Begin coding

---

## Error Handling

**If any check fails critically:**
- 🛑 Stop workflow
- 📋 Show clear error message
- 💡 Provide actionable resolution steps
- ⏸️ Wait for user to resolve issue

**If checks warn but don't block:**
- ⚠️ Show warning
- ❓ Ask user for confirmation
- ✅ Respect user's decision to proceed or fix

**If user explicitly bypasses:**
- ⚠️ Log that checks were skipped
- ✅ Proceed with caution
- 💡 Remind about best practices

---

## Future Enhancements

Potential additions:
- **Dependency check**: Verify dependencies are installed
- **Build status**: Check if project builds successfully
- **Test status**: Verify tests pass before modifications
- **Merge conflicts**: Detect unresolved merge conflicts
- **Stale branch**: Warn if branch is far behind develop/main