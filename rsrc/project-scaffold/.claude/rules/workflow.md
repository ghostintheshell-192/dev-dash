# Workflow

## Session Start

At the beginning of every session, before starting any work:

1. **Read the latest handoff** in `.memory-bank/` (the most recent `.md` file by date in the filename)
2. **Read any linked files** referenced in the handoff (e.g., progress specs, related notes)
3. **Cross-reference** with `memory/MEMORY.md` for stable project facts (if present)

This ensures continuity between sessions. The `.memory-bank/` contains detailed
session diaries (what was done, why, what's next), while `memory/MEMORY.md`
(when present) holds a compact index of stable facts.

## Session End

**Write a handoff note before ending any session.** This is non-negotiable: the
`.memory-bank/` diary is the primary continuity mechanism between sessions, and
skipping it breaks that continuity.

When the user signals end of session — in any form, in any language — invoke
the `session-handoff` skill **before** replying farewell.

The handoff lives in `.memory-bank/` with filename `YYYY-MM-DD-HHmm-<slug>.md`
and structure **Done / Next / Notes**. If the session touched multiple branches
or merges, include commit hashes and branch names so the next session can resume
git state without hunting. If anything was deferred or flagged for later,
capture it in **Next** so it does not get lost.

Do not rely only on the `SessionEnd` hook in `.claude/settings.json` — that
archives the raw transcript, it does not produce a semantic handoff.

---

## Git Workflow

**Branch strategy:**

- `main`: releases only
- `develop`: default branch for development
- `feature/*`, `fix/*`, `docs/*`, `refactor/*`, `experiment/*`, `chore/*`: task branches

**NEVER work on `main` or `develop` directly.** When branch protection is
configured (see `.githooks/`), the pre-commit hook blocks direct commits on
these branches. Merge commits (`git merge --no-ff`) are explicitly allowed —
that is the supported path to land work.

**Typical workflow:**

```bash
# Start new feature
git checkout develop
git pull origin develop
git checkout -b feature/task-name

# Work and commit
git add <files>
git commit -m "feat: descriptive message"

# Merge when complete
git checkout develop
git merge --no-ff feature/task-name
git push origin develop
```

### Collaboration convention

Claude creates task branches, commits, and merges to `develop` once the work
is complete. The user runs `git push origin develop` manually. Git commands
are confirmed one at a time rather than added to the permissions allowlist —
the confirmation prompt is the deliberate checkpoint to re-read the diff
before it lands.

### Spec lifecycle automation

If `.githooks/` ships the spec-workflow hooks, spec files are moved between
`specs/{planned,in-progress,implemented}/` based on git activity:

- **`post-checkout`**: on `git checkout -b {feature,fix,docs,refactor,experiment}/<name>`,
  the matching spec in `specs/planned/<name>.md` is moved to `specs/in-progress/`
  and its frontmatter `status` field is updated.
- **`post-merge`**: on no-conflict merges into `develop` (the normal case —
  git creates those commits directly and pre-commit does NOT run), the merged
  branch is parsed from the merge commit subject and the matching spec is
  moved to `specs/implemented/` and staged. The hook then prints the
  one-liner to fold it in (`git commit --amend --no-edit`) — run it right
  after the merge; amending from inside the hook is impossible because git
  still holds MERGE_HEAD while post-merge runs.
- **`pre-commit.d/05-spec-workflow`**: covers the complementary case only —
  conflicted merges concluded manually via `git commit`, where `MERGE_HEAD`
  still exists and pre-commit does run.

The script at `.development/scripts/spec-workflow.py` is the shared backend;
missing specs, unknown branch prefixes, and non-merge commits all exit silently.
Spec filenames may carry the branch prefix folded in (`feature/x` matches
`feature-x.md`).

### Automation entry points

Hooks and CI are **generic orchestrators**: they contain no stack-specific
commands. All stack knowledge lives in standard entry points under
`.development/automation/`:

| Entry point | Contract |
| ----------- | -------- |
| `build.sh [preset]` | Build the project; exit != 0 on failure |
| `test.sh [preset]` | Run tests; "no tests yet" is a declared no-op |
| `format-check.sh [files...]` | Verify formatting; no-op without a formatter config |
| `format-fix.sh [files...]` | Apply formatting |
| `docs-update.sh` | Regenerate ARCHITECTURE.md, INDEX.md, tech-debt index |

No args = act on everything. Multi-stack knowledge (which command for which
part of the tree) belongs *inside* the entry point, never in hooks or CI.

### Hook activation (once per clone)

```bash
bash .development/automation/bootstrap.sh
```

Sets `core.hooksPath .githooks` locally (overrides any global hooksPath),
verifies prerequisites, and makes hooks/entry points executable. Without
this, branch protection and the project hooks are NOT active.

## Investigation & Analysis Workflow

When analyzing tech-debt, bugs, or investigating issues:

1. **Read the issue description** — Understand the problem
2. **Read `.development/ARCHITECTURE.md`** — Find relevant files using:
   - Project Tree (file index with descriptions)
   - Layer Overview (understand dependencies)
   - Related ADRs (architectural context)
3. **Read files in logical order** — Follow layer structure
4. **Report findings** — Summary of what you found and where

**Always read ARCHITECTURE.md before exploring code** — it's your navigation map.

## Quick Commands

```bash
# Build (entry point — stack knowledge lives inside the script)
.development/automation/build.sh

# Run the app
{RUN_COMMAND}

# Tests
.development/automation/test.sh

# Format check
.development/automation/format-check.sh
```
