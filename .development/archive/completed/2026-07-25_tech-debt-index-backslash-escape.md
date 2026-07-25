---
type: bug
priority: low
status: resolved
discovered: 2026-07-03
resolved: 2026-07-25
related: []
related_decision: null
report: null
---

# update-tech-debt-index.py crashes on backslashes in issue titles

## Problem

`update-tech-debt-index.py` (both the dev-dash copy in `.development/scripts/`
and the scaffold copy in `rsrc/project-scaffold/.development/scripts/`) crashes
with `re.error: bad escape \s` when any tech-debt issue title contains a
backslash sequence. Since the script runs from a SessionStart hook and from
`docs-update.sh`, the crash silently breaks the tech-debt index regeneration.

## Analysis

In `update_readme()` the freshly generated section is passed as the
*replacement string* of `re.sub()`:

```python
new_content = re.sub(pattern, new_section.rstrip() + '\n', content, flags=re.DOTALL)
```

`re.sub` parses the replacement for escape sequences (`\1`, `\g<name>`, …), so
any literal backslash coming from an issue title — e.g. a title mentioning a
regex like `\s+` — is interpreted and blows up. Found in the wild in
government-feed, whose issue "Pre-commit hook: grep doesn't support `\s+`
regex" triggered exactly this.

## Possible Solutions

- **Option A**: pass a callable as replacement — `re.sub(pattern, lambda _:
  new_section.rstrip() + '\n', content, flags=re.DOTALL)`. Callables bypass
  escape parsing entirely. Minimal, no behavior change.
- **Option B**: `re.escape()` the replacement — wrong tool (escapes for
  *patterns*, not replacements) and mangles output.
- **Option C**: string-slice the section boundaries instead of re.sub —
  more invasive, no added value here.

## Recommended Approach

Option A. Already applied and verified in government-feed
(`.development/scripts/update-tech-debt-index.py`, commit `5411b21` on its
develop): fix both dev-dash copies and the scaffold source of truth so newly
scaffolded projects don't inherit the bug.

## Notes

Discovered on 2026-07-03 while porting the scaffold to government-feed: the
very first real-world run of the script hit the crash.

## Resolution (2026-07-25)

Option A applied to both copies (scaffold source of truth and the dev-dash live
copy), converging on the government-feed fix: the replacement is now a callable,
`re.sub(pattern, lambda _: new_section.rstrip() + '\n', content, ...)`, which
bypasses escape parsing entirely.

Verified in two steps. First the mechanism in isolation: the old form raises
`re.error: bad escape \s`, the new one does not. Then end to end — a temporary
issue titled with a literal backslash sequence, script run, exit 0, no
exception, title rendered with the backslash intact. Issue removed and the index
regenerated with no residue. Repeated against a throwaway copy of the scaffold
scripts, same outcome.

## Related Documentation

- **Code Locations**: `.development/scripts/update-tech-debt-index.py`
  (`update_readme()`, the `re.sub` call);
  `rsrc/project-scaffold/.development/scripts/update-tech-debt-index.py` (same)
- **Reference fix**: government-feed repo, same file, lambda replacement

---

📍 **Investigation Note**: Read [ARCHITECTURE.md](../ARCHITECTURE.md) to locate relevant files and understand the architectural context before starting your analysis.
