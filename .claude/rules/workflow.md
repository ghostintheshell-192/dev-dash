# Workflow

## Session Start

At the beginning of every session, before starting any work:

1. **Read the latest handoff** in `.memory-bank/` (the most recent `.md` file by date in the filename)
2. **Read any linked files** referenced in the handoff (e.g., progress specs, related notes)
3. **Cross-reference** with `memory/MEMORY.md` for stable project facts

This ensures continuity between sessions. The `.memory-bank/` contains detailed session diaries (what was done, why, what's next), while `memory/MEMORY.md` holds a compact index of stable facts.

## Session End

**Write a handoff note before ending any session.** This is non-negotiable: the
`.memory-bank/` diary is the primary continuity mechanism between sessions, and
skipping it breaks that continuity.

When the user signals end of session — in any form, in any language — invoke
the `session-handoff` skill **before** replying farewell. Recognize phrases
like "fermiamoci", "è tardi", "chiudiamo", "continuiamo domani", "ciao", "/exit",
"/clear", explicit requests for a summary, and equivalent signals.

The handoff lives in `.memory-bank/` with filename `YYYY-MM-DD-HHmm-<slug>.md`
and structure **Done / Next / Notes** (see recent entries). If the session
touched multiple branches or merges, include commit hashes and branch names
so the next session can resume git state without hunting. If anything was
deferred or flagged for later, capture it in **Next** so it does not get lost.

Do not rely only on the `SessionEnd` hook in `.claude/settings.json` — that
archives the raw transcript, it does not produce a semantic handoff.

---

## Git Workflow

**Branch strategy:**

- `main`: releases only
- `develop`: default branch for development
- `feature/*`, `fix/*`, `docs/*`, `refactor/*`, `experiment/*`, `chore/*`: task branches

**NEVER work on `main` or `develop` directly.** Both are branch-protected: the
pre-commit hook `.githooks/pre-commit.d/00-branch-protection` blocks direct
commits. Merge commits (`git merge --no-ff`) are explicitly allowed on these
branches — that is the supported path to land work.

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

On this project, Claude creates task branches, commits, and merges to `develop`
once the work is complete. The user runs `git push origin develop` manually.
Git commands are confirmed one at a time rather than added to the permissions
allowlist — the confirmation prompt is the deliberate checkpoint to re-read the
diff before it lands.

### Spec lifecycle automation

The repo ships hooks that move spec files between `specs/{planned,in-progress,implemented}/`
based on git activity:

- **`post-checkout`**: on `git checkout -b {feature,fix,docs,refactor,experiment}/<name>`,
  the matching spec in `specs/planned/<name>.md` is moved to `specs/in-progress/`
  and its frontmatter `**Status**` field is updated.
- **`pre-commit.d/05-spec-workflow`**: on merge commits into `develop`, the branch
  name is parsed and the matching spec is moved to `specs/implemented/` and
  staged as part of the merge commit.

The script at `.development/scripts/spec-workflow.py` is the shared backend;
missing specs, unknown branch prefixes, and non-merge commits all exit silently.
Activation requires `git config core.hooksPath .githooks` (done once per clone).

## Investigation & Analysis Workflow

When analyzing tech-debt, bugs, or investigating issues:

1. **Read the tech-debt/issue description** - Understand the problem
2. **Read `.development/ARCHITECTURE.md`** - Find relevant files using:
   - Project Tree (file index with descriptions)
   - Layer Overview (understand dependencies)
   - Related ADRs (architectural context)
3. **Read files in logical order** - Follow layer structure (UI → Core ← Infrastructure)
4. **Report findings** - Summary of what you found and where

**Always read ARCHITECTURE.md before exploring code** - it's your navigation map.

## Quick Commands

```bash
# Configure + build (Linux Debug)
cmake --preset linux-debug -S poc
cmake --build --preset linux-debug

# Run the PoC
./poc/build/linux-debug/src/dev-dash-poc

# Tests: TBD (no test target yet; will arrive once core/services land in app/)

# Format check: TBD (no .clang-format at root yet; the 02-clang-format hook
# is dormant until the config file is added)
```
