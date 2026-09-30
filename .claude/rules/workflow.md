# Workflow

## Session Start

At the beginning of every session, before starting any work:

0. **Make sure the journal is there.** Handoffs and transcripts live in a
   private repository of their own, `ghostintheshell-192/dev-dash-memory`,
   cloned at `.memory-bank/journal/` and ignored by dev-dash. On a local clone
   it is set up once per machine. In a **cloud session** it is not: attach the
   repository (push access) and clone only the handoffs:

   ```bash
   git clone --filter=blob:none --sparse \
       https://github.com/ghostintheshell-192/dev-dash-memory .memory-bank/journal
   git -C .memory-bank/journal sparse-checkout set handoffs
   ```

1. **Read the latest handoff** in `.memory-bank/journal/handoffs/` (the most recent `.md` file by date in the filename)
2. **Read any linked files** referenced in the handoff (specs, idea notes,
   related handoffs)
3. **Cross-reference** with `memory/MEMORY.md` for stable project facts

This is the continuity mechanism between sessions and it is not optional. The
handoffs are session diaries — what was done, why, what is next, in priority
order. `MEMORY.md` is a compact index of stable facts; it does **not** carry the
task ordering or the per-task caveats, so starting from it alone loses them.

## Session End

**Write a handoff note before ending any session.** This is non-negotiable: the
`.memory-bank/` diary is the primary continuity mechanism between sessions, and
skipping it breaks that continuity.

When the user signals end of session — in any form, in any language — invoke
the `session-handoff` skill **before** replying farewell. Recognize phrases
like "fermiamoci", "è tardi", "chiudiamo", "continuiamo domani", "ciao", "/exit",
"/clear", explicit requests for a summary, and equivalent signals.

The handoff lives in `.memory-bank/journal/handoffs/` with filename `YYYY-MM-DD-HHmm-<slug>.md`
and structure **Done / Next / Notes** (see recent entries). If the session
touched multiple branches or merges, include commit hashes and branch names
so the next session can resume git state without hunting. If anything was
deferred or flagged for later, capture it in **Next** so it does not get lost.

After writing it, **commit and push the journal** (`git -C .memory-bank/journal
add -A && git -C .memory-bank/journal commit -m "..." && git -C
.memory-bank/journal push`). In a cloud session this is the only way the
handoff survives: the container is thrown away.

Do not rely only on the `SessionEnd` hook in `.claude/settings.json` — that
archives the raw transcript into `.memory-bank/journal/sessions/` (and pushes
it), it does not produce a semantic handoff.

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

### Automation entry points (ADR-012)

Hooks and CI are **generic orchestrators**: they contain no stack-specific
commands. All stack knowledge (CMake, ctest, clang-format) lives in standard
entry points under `.development/automation/`:

| Entry point | Contract |
| ----------- | -------- |
| `build.sh [preset]` | Build the project; exit != 0 on failure |
| `test.sh [preset]` | Run tests; "no tests yet" is a declared no-op |
| `format-check.sh [files...]` | Verify formatting; no-op without `.clang-format` |
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

### Keeping CURRENT-STATUS.md current

`.development/CURRENT-STATUS.md` is written by hand; no hook regenerates it. When a
spec is completed (it moves to `specs/implemented/`), update it **on the same branch,
before the merge into `develop`**: the phase summary, a *Recent Work* entry, and any
roadmap item it closes.

## Investigation & Analysis Workflow

When analyzing tech-debt, bugs, or investigating issues:

1. **Read the tech-debt/issue description** - Understand the problem
2. **Read `.development/ARCHITECTURE.md`** - Find relevant files using:
   - Project Tree (file index with descriptions — derived from source, so it is
     the authority on what exists)
   - Layer Overview (what each layer is *for*; it deliberately does not list
     classes or implementation status — see `coding-standards.md` for the layer
     dependency rules, and `CURRENT-STATUS.md` for what is implemented)
   - Related ADRs (architectural context)
3. **Read files in logical order** - Follow layer structure (UI → Core ← Infrastructure)
4. **Report findings** - Summary of what you found and where

**Always read ARCHITECTURE.md before exploring code** - it's your navigation map.

## Quick Commands

```bash
# Build (entry point — preferred; defaults to linux-debug preset)
.development/automation/build.sh

# Run the app
./app/build/linux-debug/src/dev-dash

# Tests (no-op until the test target lands, feature-release-readiness)
.development/automation/test.sh

# Format check (no-op until .clang-format exists at root)
.development/automation/format-check.sh
```
