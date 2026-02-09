# Git Workflow and Development Commands

## Branch Strategy

- **main**: Production-ready code
- **feature/**: New features (e.g., `feature/user-authentication`)
- **fix/**: Bug fixes (e.g., `fix/login-error`)
- **refactor/**: Code refactoring (e.g., `refactor/database-layer`)

## Development Workflow

### Starting New Work

```bash
# Create feature branch from main
git checkout main
git pull origin main
git checkout -b feature/descriptive-name
```

### During Development

```bash
# Commit frequently with clear messages
git add [specific files]
git commit -m "feat: Add user authentication

- Implement JWT token generation
- Add login/logout endpoints
- Include unit tests

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"
```

### Completing Work

```bash
# Push to remote
git push origin feature/descriptive-name

# Create pull request (if applicable)
gh pr create --title "Add user authentication" --body "$(cat <<'EOF'
## Summary
- Implemented JWT authentication
- Added login/logout endpoints
- Added unit tests

## Test Plan
- [ ] Test login with valid credentials
- [ ] Test login with invalid credentials
- [ ] Test token expiration
- [ ] Test logout

🤖 Generated with [Claude Code](https://claude.com/claude-code)
EOF
)"
```

## Commit Message Format

```
<type>: <subject>

<body>

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

**Types**: `feat`, `fix`, `docs`, `refactor`, `test`, `chore`

## Session Handoffs

At end of session, use `/handoff` command to create handoff note in `.memory-bank/`.
