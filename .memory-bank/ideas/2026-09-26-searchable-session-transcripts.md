---
captured: 2026-09-26
status: open
context: "conversation about dev-dash's future, right after the transcript redaction work (redact_transcript.py wired into session-archive.py)"
tags: [session-handoff, transcripts, search, retrieval, memory-bank]
---

# Make archived session transcripts searchable by Claude

## What this is

`journal/README.md` already describes the transcripts as *"a backup, to search
for what did not make it into a handoff"* — but nothing makes that search
possible today short of reading megabytes of JSONL. The idea is to turn
`journal/sessions/` into something Claude can query cheaply: a keyword index
first, optionally semantic retrieval later (RAG in the broad sense — retrieve
the relevant fragments, then reason over them).

Scale at capture time: ~29 transcripts, mostly 1–2.8 MB each, ~40 MB total,
plus 24 handoffs of 2–13 KB. Handoffs and sessions share the same timestamp
naming, so each handoff can be linked to the transcript it came from.

## Why it deserves attention

Handoffs and ADRs record what was *decided*. What they tend to lose is what was
*tried and abandoned* — dead ends, rejected approaches, the exact failure that
motivated a pivot. That is the one class of question only the transcripts can
answer, and it is the kind of context dev-dash exists to preserve.

Honest doubt, stated by Valentina at capture: not sure how useful it is in
practice. If the questions that come up are mostly "why did we decide X", the
better fix is in what the handoff skill captures, not in searching afterwards.

## Shape, if it is ever picked up

1. **Normalise** — from each JSONL keep user messages and assistant text;
   collapse tool calls to one line ("read X", "ran Y"). Most of the raw volume
   is tool output and is noise for retrieval; the signal is likely under 10%.
2. **Chunk by exchange** (question + answer), each chunk carrying date,
   session id and linked handoff.
3. **Index** in SQLite FTS5 (BM25): one file, no service, fast enough at this
   scale. Embeddings (local, multilingual — the transcripts mix Italian and
   English, e.g. via a SQLite vector extension) only if keyword search
   demonstrably misses things.
4. **Expose to Claude** as a tool it calls on demand (script + skill, or a small
   MCP server) — agentic retrieval, not automatic context injection. Results
   return a few hundred tokens with references, not whole files.
5. **Two tiers** — search handoffs first, drill into the linked transcript only
   when the detail is needed.

## Constraints already known

- **Redaction is solved upstream**: `session-archive.py` runs
  `redact_transcript.py` before commit, so the index inherits masked content.
  The repo must stay private regardless (detection is by shape).
- **Staleness**: transcripts are history, not truth. Results must show their
  date prominently, and the skill must tell Claude that ADRs and specs win over
  anything found in a transcript.
- **Cloud sessions** clone the journal sparse, handoffs only — so the index is
  a local-machine capability unless that changes. The handoff tier could still
  work everywhere.

## Minimal next step

Prototype step 1 alone on one large transcript and measure what survives
normalisation. That number decides whether the rest is worth building — and
the normalised text is useful by itself, even without an index.
