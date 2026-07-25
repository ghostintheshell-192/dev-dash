---
captured: 2026-05-14
status: open
context: "Diagnosing recurring 'Dispositivo o risorsa occupata' warnings on .git/config during git operations inside Claude Code sessions"
tags: [feedback, claude-code, sandbox, dx]
---

# Feedback to Anthropic: noisy bind-mount RO warnings on `.git/config`

## What

Inside a Claude Code session, routine git operations that update `.git/config`
(e.g. `git branch -d` on multiple branches, `git config user.email ...`) print
warnings like:

```
error: impossibile scrivere il file di configurazione .git/config:
Dispositivo o risorsa occupata
warning: Aggiornamento del file di configurazione non riuscito
```

The operation **completes successfully** — git falls back to its lock-file +
atomic-rename path. But the warnings are loud and recurring, and look like
real errors to anyone who doesn't know what a bind mount is.

## Why it happens

The Claude Code sandbox bind-mounts a list of "sensitive" files as read-only,
including `.git/config` (and `.git/hooks`, `.claude/settings*.json`, home
dotfiles like `.bashrc`, `~/.gitconfig`, etc.). When git attempts a direct
write to `.git/config`, the kernel returns `EBUSY` because the mount is RO.
Git then retries via its standard pattern (write `.git/config.lock`, then
`rename()`), which succeeds because rename acts on the parent directory's
inode, not on the bind-mounted file.

Confirmed by:

```bash
mount | grep .git/config
# /dev/mapper/vg-data on /data/repos/dev-dash/.git/config type ext4 (ro,...)
```

The bind mount is **hardcoded in the sandbox runtime**, not configurable from
`settings.json`. The sandbox section in `~/.claude/settings.json` exposes
allowWrite / denyWrite / denyRead, but the bind-mount list for sensitive
files is internal.

## Why it deserves attention

- DX papercut: every git session that touches branch state produces dozens of
  these warnings, indistinguishable at a glance from real errors.
- The protection itself is reasonable, but the surfacing is bad: the user
  sees a failure mode that the system has already silently recovered from.
- Risk of harmful "fixes": a less informed user might try to delete the
  lock file, `chmod` `.git/config`, or otherwise act on a non-problem.

## Suggested directions for Anthropic

1. **Don't bind-mount `.git/config` inside the primary cwd**: the cwd is
   already a writable scope, so the protection adds noise without much value
   for files the user is actively working on.
2. **Or: intercept and route through the rename-atomic path silently** when
   the direct write would fail, suppressing the intermediate warning.
3. **Or: at minimum, document** that these warnings are benign in `claude
   --help` / docs, so users don't waste time investigating.

## Next step if/when I file it

- Search existing issues on `anthropics/claude-code` for "bind mount", "git
  config", "Device busy" — may already be open.
- File at https://github.com/anthropics/claude-code/issues with the title
  *"Sandbox bind-mount RO on .git/config causes noisy 'Device or resource
  busy' warnings during routine git operations"* and include the `mount`
  output as evidence.
