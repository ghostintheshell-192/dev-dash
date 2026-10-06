# Idea Capture Rule

Tangential ideas are parked in `.memory-bank/ideas/`, one note each, without
derailing the current work. The procedure (capture, review, promote, and the note
format) lives in the `idea-capture` skill; this rule only says when to use it.

## When to activate

**Explicit user triggers** (typical phrases, in any language):

- *"let's note it and move on"*
- *"save it for later"*
- *"not now, park it"*
- *"interesting but out of scope"*
- *"take a note and continue"*
- *"what ideas do we have parked?"* (review)

**Proactive trigger from Claude**: when you sense a tangential thread that has its
own merit but risks derailing the main work, **ask** *"want me to note it as an
idea to explore?"* instead of following the tangent.

**Sub-agent reports**: when a delegated agent returns an `Ideas` section, filter it
and offer what survives to the user. Sub-agents report ideas; only the main session
writes the notes.
