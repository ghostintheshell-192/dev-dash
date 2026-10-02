---
type: bug
priority: low
status: open
discovered: 2026-10-01
resolved: null
related: []
related_decision: 017-code-graph-extraction.md
report: null
upstream: tree-sitter-cpp
upstream_link: null
---

# tree-sitter-cpp: `= {}` default argument parsed as an error

## Problem

tree-sitter-cpp 0.23.4 marks a parameter whose default argument is an empty
brace initializer as a syntax error, e.g.

```cpp
core::Snapshot SaveExplicit(const core::Project& project,
                            std::string_view name,
                            std::string_view description = {});
```

The code compiles; the code graph lists the file as "read in part".

## Analysis

Seen in `services/snapshot_service.h`, `services/cpp_class_extractor.cpp` and
the test helpers of dev-dash. tree-sitter recovers around the error and no
member is lost (checked against the members of `SnapshotService`), so the
effect is only a false "read in part".

Checked 2026-10-02: 0.23.4 is still the latest tag, and `master` (`c009222`,
2026-09-17) parses the same declarations with the same `MISSING` node
(`snapshot_service.h:22`, `cpp_class_extractor.h:57`). Not fixed upstream:
worth reporting.

## Possible Solutions

- **Option A**: report it upstream with the snippet above (or update the pin
  if a newer release fixes it).

## Recommended Approach

Option A.
