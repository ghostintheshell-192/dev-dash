# Architecture Decision Records

This folder records the key architectural decisions made for the project.
Start a new ADR by copying [`_TEMPLATE.md`](_TEMPLATE.md).

## Format

Each ADR carries a frontmatter block followed by prose sections. See
`_TEMPLATE.md` for the canonical layout.

**Frontmatter** (in this order):

| Field | Required | Notes |
| ----- | -------- | ----- |
| `**Date**` | yes | Date the decision was made (`YYYY-MM-DD`). |
| `**Status**` | yes | One of: `Proposed`, `Accepted`, `Superseded`, `Deprecated`. |
| `**Impact**` | yes | One of: `critical`, `high`, `medium`, `low` (see scale below). |
| `**Summary**` | yes | One sentence — *what* the ADR decides. Extracted verbatim by the generators. |
| `**Supersedes**` / `**Sub-decision of**` / `**Superseded by**` | optional | Cross-links, placed after `Summary`. |

**Body sections**: `## Context`, `## Decision`, `## Rationale`,
`## Consequences`.

## Impact scale

`Impact` controls how far a decision reaches into the agent's context — it is
the contract behind two generators (`generate-claude-config.sh`,
`generate-architecture.sh`):

| Level | Meaning | Reaches context as |
| ----- | ------- | ------------------ |
| `critical` | Rule that must never be violated. | Loaded **every session** into `key-decisions.md`, flagged as a constraint. |
| `high` | Context that shapes ongoing decisions. | Loaded **every session** into `key-decisions.md`. |
| `medium` | Relevant, consult on demand. | Indexed only (title + Summary in the ARCHITECTURE.md list). |
| `low` | Historical or narrow record (e.g. a superseded ADR). | Indexed only. |

The auto-load threshold is **`Impact ≥ high`**: `critical` + `high` decisions
are digested into `.claude/key-decisions.md`, which `.claude/CLAUDE.md`
pulls in via `@include` at session start.

## Naming convention

`NNN-short-description.md` — e.g. `010-architecture-design.md`.

## Index

The up-to-date catalogue of ADRs — title, `Impact`, and one-line Summary —
is **auto-generated** in
[`.development/ARCHITECTURE.md`](../../ARCHITECTURE.md) under *Key Decisions*,
produced by `generate-architecture.sh`. It is not duplicated here, to avoid
drift.
