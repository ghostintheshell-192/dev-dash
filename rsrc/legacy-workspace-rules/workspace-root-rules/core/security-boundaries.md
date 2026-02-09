# Security Boundaries

**MANDATORY rules for file system access and destructive operations.**

---

## File System Access Rules

### NEVER access or modify

- `/data/secrets/**` - Secrets directory (off-limits for Claude Code)
- `~/.ssh/id_*` - SSH private keys (read-only via git operations only)
- `~/.git-credentials` - Git credentials (now in `/data/secrets/git/credentials`, managed automatically)
- `*.key`, `*.pem`, `*.p12`, `*.pfx` - Private key files
- `*secret*`, `*credential*`, `*password*` files (unless `.example` suffix)

### Read-only access

- `/etc/**` - System configurations
- `~/.gitconfig` - Git configuration (read only, never modify manually)
- `~/.ssh/known_hosts` - SSH known hosts
- `/data/backups/**` - Backup archives (read only)

### Write access authorized

- `/data/repos/**` - All repository work (primary workspace)
- `/tmp/claude-*` - Temporary files for Claude operations
- Project-specific `.vscode/`, `.claude/` directories

---

## Destructive Operations - Always Confirm

Before executing these commands, **ALWAYS ask user confirmation**:

- `rm -rf` on any directory (especially outside `/tmp/`)
- `git push --force` or `git push -f`
- `git reset --hard` (data loss risk)
- `chmod` on files outside `/data/repos/`
- `mv` or `cp` operations involving `/data/secrets/` or `/data/backups/`
- Git operations on `main` or `develop` branches without proper feature branch
- Deletion of `.git/` directories or git configuration files

---

## Secrets Handling Best Practices

### Detection and Prevention

- Global pre-commit hook active: `/data/repos/.git-hooks/pre-commit`
- Hook blocks commits containing API keys, passwords, tokens, private keys
- All repositories protected via `git config --global core.hooksPath`

### When secrets are needed

- **Never read** files from `/data/secrets/` (not needed for development)
- **If user provides API key**: Suggest storing in `/data/secrets/api-keys/project-name/`
- **Config files**: Always use placeholders like `api_key: null` or `api_key: ${ENV_VAR}`
- **Environment variables**: Prefer environment-based secrets over hardcoded values
- **Never commit**: Verify `.gitignore` includes secret patterns before `git add`

### Global .gitignore protection

- Location: `/data/repos/.gitignore`
- Protects: `.env`, `*.key`, `*.pem`, `*secret*`, `*credential*` patterns
- Applies to all new repositories in `/data/repos/`

---

## Safe Defaults and Pre-Flight Checks

### File operations

- **Always run `pwd`** before `mv`, `rm`, `cp` involving multiple paths
- Verify parent directory exists before creating subdirectories
- Check file existence before moving/copying to avoid overwrites

### Git operations

- Verify current branch before merge/rebase (avoid accidental main/develop changes)
- Check git status before force operations
- Confirm remote before push --force

### New projects

- Create with `.env.example` (template) and `.env` in `.gitignore`
- Add project-specific `CLAUDE.md` using template from `/data/repos/rules/templates/`
- Initialize git with appropriate branch structure (develop as default)

---

## Security Infrastructure

### Directories

```text
/data/secrets/          # Credentials storage (700 permissions, owner-only)
├── ssh/               # SSH keys backups
├── api-keys/          # API keys by project
├── git/               # Git credentials
└── gpg/               # GPG keys backups

/data/backups/         # Backup archives (755 permissions)
├── repos/             # Repository backups
├── secrets/           # Encrypted secrets backups
├── config/            # Configuration backups
└── projects/          # Project-specific backups
```

### Git configuration

- Credentials: `~/.gitconfig` → `credential.helper = store --file=/data/secrets/git/credentials`
- Hooks: `core.hooksPath = /data/repos/.git-hooks`
- Signing: GPG signing enabled (`commit.gpgSign = true`)

### Emergency contacts

- Hook documentation: `/data/repos/.git-hooks/README.md`
- Secrets structure: `/data/secrets/README.md`
- Migration log: Check `/data/repos/MIGRATION-LOG-*.md` for setup history

---

*See also: `Vault@Claude/buone-pratiche.md` for self-imposed guidelines*
