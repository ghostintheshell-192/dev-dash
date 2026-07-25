---
type: bug
priority: medium
status: open
discovered: 2026-07-05
related: [architecture-layer-overview-stale-prose.md]
related_decision: null
report: null
---

# generate-architecture.sh: eval re-expands unquoted globs against the CWD

## Problem

The scaffold's `generate-architecture.sh` produces an **empty project tree**
(silently, `Stats: 0 files`) in any project that has source files matching
`FILE_GLOBS` **in the repository root** — the directory the script is invoked
from. The failure is silent because `find` errors are swallowed by
`2>/dev/null`, so the generated `ARCHITECTURE.md` looks "successfully
regenerated" while its tree section is blank.

Discovered while applying the scaffold to **synedria** (Next.js): its root
holds `next.config.ts` and `next-env.d.ts`. It has never surfaced in `dev-dash`
(C++) or `government-feed` because neither keeps source files in the root — so
this is a **latent bug in every project**, not a synedria-specific one. It
would trigger the first time either project adds, say, a `main.py` or a build
script matching its globs to the root.

## Analysis

Root cause: `build_find_name_args` emits the `find` name predicates as a
**string with unquoted globs** (`-name *.ts -o -name *.tsx`), which is then run
through `eval`:

```bash
eval "find \"$dir\" -maxdepth 1 -type f \( $find_args \) -print0 2>/dev/null"
```

`eval` re-parses the string, and the shell performs **pathname expansion on
`*.ts` against the current working directory** (the repo root, where the
script runs). Behaviour depends entirely on what sits in the root:

- **No matching file in root** → glob stays literal → `find -name '*.ts'`
  works. (dev-dash, government-feed today.)
- **One matching file** → glob expands to that filename → `find` matches only
  that exact name, silently missing every other source file.
- **Two or more matching files** → glob expands to multiple words →
  `find ... -name next.config.ts next-env.d.ts ...` is a malformed expression
  (`-name` takes exactly one argument) → `find` aborts → **0 files**. (synedria.)

`count_stats` has the same defect (it builds `prune_args` as a string and
`eval`s it too).

Proven empirically: with CWD at the synedria root, the eval'd command expands to
`find src -maxdepth 1 -type f \( -name next.config.ts next-env.d.ts -o -name *.tsx \)`
and returns nothing; wrapping the same eval in a `set -f` (noglob) subshell
restores the correct output.

The older, hand-written `generate-architecture.sh` variant (still in synedria)
avoids the whole class of bug by passing `"$glob"` **quoted** straight to
`find`, per glob, with no `eval` — the shell never gets a chance to expand it.

## Possible Solutions

- **Option A — array of find args, no eval (recommended)**: have
  `build_find_name_args` populate a bash **array** instead of a string, and
  splice it quoted into `find`. Array expansion does not glob, so patterns
  reach `find` literally. Keeps the "single find with `-o`" structure.
- **Option B — `set -f` around the eval sites**: run each `eval` in a
  `( set -f; eval "…" )` subshell to disable pathname expansion. Minimal (2
  lines) but keeps `eval` and its quoting hazards.
- **Option C — per-glob quoted find loop**: adopt synedria's hand-written
  approach (one `find` per glob, `"$glob"` quoted). Simplest to reason about;
  slightly more process spawns.

## Recommended Approach

**Option A.** It removes `eval` entirely (the root of the fragility), stays
close to the current structure, and is robust regardless of what lives in the
repo root.

```bash
# Populate a global array of find -name args with -o between each pattern.
# Array form (not a string through eval) keeps globs literal — the shell
# never re-expands them against the CWD.
build_find_name_args() {
    FIND_NAME_ARGS=()
    local first=1
    for glob in "${FILE_GLOBS[@]}"; do
        if [ $first -eq 1 ]; then
            FIND_NAME_ARGS+=(-name "$glob"); first=0
        else
            FIND_NAME_ARGS+=(-o -name "$glob")
        fi
    done
}

# process_directory — no eval:
build_find_name_args
while IFS= read -r -d '' file; do
    files+=("$file")
done < <(find "$dir" -maxdepth 1 -type f \( "${FIND_NAME_ARGS[@]}" \) -print0 2>/dev/null | sort -z)

# count_stats — de-eval the prune list too:
build_find_name_args
local prune=()
for excl in "${EXCLUDE_DIRS[@]}"; do prune+=( -path "*/$excl/*" -prune -o ); done
... < <(find "$source_dir" "${prune[@]}" \( "${FIND_NAME_ARGS[@]}" \) -type f -print0 2>/dev/null)
```

## Notes

- The scaffold template is the source of truth for propagation:
  `rsrc/project-scaffold/.development/scripts/generate-architecture.sh`.
- The **live copies** in `dev-dash` and `government-feed`
  (`.development/scripts/generate-architecture.sh`) carry the same latent bug
  and should be re-synced from the fixed template.
- **synedria is already safe**: it kept its hand-written quoted-glob variant
  (Option C in spirit) plus the scaffold's richer ADR-list function. No action
  needed there beyond eventually converging on the fixed template.
- Consider a guard so silent failure can't recur: if the tree comes back empty
  while `SOURCE_DIRS` is non-empty and populated, emit a warning instead of
  writing a blank tree.
- **Confirmed again on `raid-sandbox` (2026-07-24)** — the first observed
  **one-matching-file** case, and the worst-behaved of the three. Its root holds
  a single `kb.js`, so `*.js` expanded to exactly one word: `find` stayed a
  *valid* expression and dutifully searched `src/` for a file literally named
  `kb.js`. No error, exit 0, `Stats: 0 files`, and a plausible-looking
  `ARCHITECTURE.md` with an empty tree. The two-or-more case at least yields a
  malformed expression; this one fails completely silently — which strengthens
  the case for the guard above. Option A was applied there and the tree went
  from 0 to 23 files, so the recommended fix is verified in practice.

## Related Documentation

- **Related Issues**: `architecture-layer-overview-stale-prose.md` — same
  script (`generate-architecture.sh`), sibling failure mode: that one makes a
  section silently *wrong*, this one can make the tree silently *empty*.
- **Code Locations**:
  - `rsrc/project-scaffold/.development/scripts/generate-architecture.sh` —
    `build_find_name_args`, `process_directory`, `count_stats`
  - `.development/scripts/generate-architecture.sh` (live copy, same defect)
- **Cross-project**: paired with the `spec-workflow.py` numeric-prefix
  tolerance patch — both are scaffold improvements surfaced by the synedria
  alignment (2026-07-04/05) and pending propagation upstream.

---

📍 **Investigation Note**: Read [ARCHITECTURE.md](../ARCHITECTURE.md) to locate relevant files and understand the architectural context before starting your analysis.
