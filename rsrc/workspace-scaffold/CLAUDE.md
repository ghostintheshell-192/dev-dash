# Claude Code - Workspace Configuration

**Domain: CODING**

This workspace contains all development repositories.

---

## Quick Reference

| Need | Location |
|------|----------|
| Bootstrap (always read first) | `.rules/bootstrap-coding.md` |
| Development principles | `.rules/core/principles.md` |
| Security boundaries | `.rules/core/security-boundaries.md` |
| Git workflow | `.rules/workflows/git.md` |
| .personal/ structure | `.rules/workflows/personal-folder.md` |
| Session workflow | `.rules/workflows/session.md` |
| Language detection | `.rules/goto.yaml` |
| User preferences | `.rules/user-preferences.yaml` |
| Coding standards | `.rules/coding-standards/*.md` |
| Session handoff | `.memory-bank/projects/[project-name].md` |

---

## Coding Standards Workflow

**ALWAYS follow this process before coding:**

1. **Read bootstrap**: `.rules/bootstrap-coding.md` (if not already loaded)
2. **Read user preferences**: `.rules/user-preferences.yaml`
3. **Detect language**: Use `.rules/goto.yaml` to match file patterns
4. **Load standards**: Read files from `.rules/coding-standards/` as indicated
5. **Pre-flight checks**: Verify git status, branch, etc.
6. **Proceed with task**

### Language Detection (goto.yaml)

| Files present | Language | Standards to load |
|---------------|----------|-------------------|
| `*.csproj`, `*.sln`, `*.cs` | C#/.NET | `general-principles.md` + `csharp-dotnet.md` |
| `pubspec.yaml`, `*.dart` | Flutter | `general-principles.md` + `flutter-dart.md` |
| `requirements.txt`, `*.py` | Python | `general-principles.md` + `python.md` |
| `package.json`, `*.ts` | TypeScript | `general-principles.md` + `javascript-typescript.md` |
| `CMakeLists.txt`, `*.cpp` | C++ | `general-principles.md` + `c-cpp.md` |

### Standards Structure

```
.rules/
├── bootstrap-coding.md       # Essential context for coding sessions
├── core/
│   ├── principles.md         # Development philosophy
│   └── security-boundaries.md # Security rules
├── workflows/
│   ├── git.md                # Branch strategy, commits
│   ├── personal-folder.md    # .personal/ structure
│   └── session.md            # Session start/end workflow
├── coding-standards/
│   ├── general-principles.md # Always load
│   └── [language].md         # Language-specific
├── goto.yaml                 # Language detection routing
├── user-preferences.yaml     # Behavior configuration
├── projects.yaml             # Auto-generated project map
└── templates/                # Project templates
```

---

## Pre-Flight Checks

Before modifying code:

1. **Git status clean?** — No uncommitted changes
2. **Correct branch?** — NOT on `main` (create feature branch if needed)
3. **Task clear?** — Ask for clarification if ambiguous
4. **Standards loaded?** — Confirm language detected

See: `.rules/preflight-checks.md` for detailed checklist.

---

## Session Handoff

For coding projects, update handoff notes at session end:

**Location**: `.memory-bank/projects/[project-name].md`

Format:
```markdown
## YYYY-MM-DD - Brief Title

**Done**:
- What was accomplished

**Next**:
- Next steps

**Notes**:
- Context for future sessions
```

---

*This workspace is fully portable. All paths are relative.*
