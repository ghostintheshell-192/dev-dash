# Repository Standards

## 1. Naming Conventions

### Repository Names

- **Always use kebab-case** for repository folder names
- Examples: `financial-tracker`, `cadenza-app`, `user-management-system`
- Avoid abbreviations unless universally understood
- Use descriptive names that indicate the project's purpose

### Internal Project Names

- Follow language-specific conventions (see `naming-mappings.yaml`)
- Repository `kebab-case` names map to internal conventions:
  - **C#**: `financial-tracker` → `FinancialTracker` (PascalCase)
  - **Python**: `financial-tracker` → `financial_tracker` (snake_case)
  - **Flutter/Dart**: `financial-tracker` → `financial_tracker` (snake_case)
  - **C/C++**: `financial-tracker` → `financial_tracker` (snake_case)

## 2. Required Files

### Mandatory Files

Every repository must contain:

- `README.md` - Project description, setup, usage
- `LICENSE` - License information
- `.gitignore` - Language-appropriate ignore patterns
- `CLAUDE.md` - Claude Code specific instructions

### Recommended Files

- `CHANGELOG.md` - Version history and changes
- `CONTRIBUTING.md` - Guidelines for contributors (if applicable)
- `docker-compose.yml` or `Dockerfile` - If containerized

## 3. Standard Directory Structure

### Common Structure (All Projects)

```
repository-name/
├── README.md
├── LICENSE
├── .gitignore
├── CLAUDE.md
├── docs/                    # Documentation
│   ├── TECHNICAL.md
│   ├── ROADMAP.md
│   └── api/                 # API documentation
├── scripts/                 # Build/deployment scripts
└── [language-specific dirs]
```

### Language-Specific Structures

#### **Flutter/Dart Projects**

```
flutter-project/
├── lib/
│   ├── models/
│   ├── services/
│   ├── controllers/
│   ├── screens/
│   ├── widgets/
│   └── utils/
├── test/
├── assets/
├── android/
├── ios/
└── pubspec.yaml
```

#### **Python Projects**

```
python-project/
├── src/
│   └── project_name/
├── tests/
├── docs/
├── requirements.txt (or pyproject.toml)
├── setup.py (if needed)
├── run.py (optional convenience script)
└── venv/ (local, not versioned)
```

**Virtual Environment Setup:**

```bash
# Create virtual environment
python -m venv venv

# Activate (Linux/Mac)
source venv/bin/activate

# Activate (Windows)
venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt

# Deactivate when done
deactivate
```

#### **C#/.NET Projects**

```
csharp-project/
├── src/
│   └── ProjectName/
├── tests/
│   ├── Unit/                    # Unit tests
│   ├── Integration/             # Integration tests
│   ├── Performance/             # Performance tests (if needed)
│   └── E2E/                     # End-to-end tests (if needed)
├── docs/
├── ProjectName.sln
└── README.md
```

**For multi-module projects:**

```
large-csharp-project/
├── src/
│   ├── Core/
│   ├── Api/
│   ├── Database/
│   └── Services/
├── tests/
│   ├── Core.Unit/
│   ├── Core.Integration/
│   ├── Api.Unit/
│   ├── Api.Integration/
│   ├── Database.Unit/
│   ├── Services.Unit/
│   └── E2E/                     # System-wide end-to-end tests
├── docs/
├── ProjectName.sln
└── README.md
```

#### **C/C++ Projects**

```
cpp-project/
├── src/
├── include/
├── tests/
├── build/ (not versioned)
├── CMakeLists.txt (or Makefile)
└── README.md
```

## 4. Version Control Standards

### Branch Naming

- `main` - Primary development branch
- `develop` - Integration branch (if using Git Flow)
- `feature/short-description` - Feature branches
- `hotfix/issue-description` - Emergency fixes
- `release/v1.2.3` - Release preparation

### Commit Message Format

```
type(scope): brief description

Extended description (if needed)

- Breaking changes noted
- Issue references: #123
```

**Types**: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

### Tagging Strategy

- Use semantic versioning: `v1.0.0`, `v1.0.1`, `v1.1.0`
- Tag all releases
- Include release notes

## 5. Documentation Standards

### README.md Template

```markdown
# Project Name

Brief description of what the project does.

## Features
- Feature 1
- Feature 2

## Installation
Step-by-step installation instructions

## Usage
Basic usage examples

## Development
How to set up development environment

## Contributing
Guidelines for contributors (if applicable)

## License
License information
```

### Documentation Rules

- All documentation in English
- Use Markdown for text documentation
- Include code examples where appropriate
- Keep documentation up-to-date with code changes

## 6. File Naming Conventions

### General Rules

- Use descriptive names
- Avoid special characters except `-`, `_`, `.`
- Follow language-specific conventions for internal files
- Use lowercase for cross-platform compatibility

### Documentation Files

- `README.md`, `LICENSE`, `CHANGELOG.md` (UPPERCASE)
- Technical docs use kebab-case: `api-reference.md`, `deployment-guide.md`

## 7. Ignore Patterns

### Standard .gitignore Categories

- Build artifacts (`build/`, `dist/`, `target/`)
- Virtual environments (`venv/`, `env/`, `.venv/`, `node_modules/`, `packages/`)
- IDE files (`.vscode/`, `.idea/`, `*.swp`)
- OS files (`.DS_Store`, `Thumbs.db`)
- Secrets (`.env`, `*.key`, `config.local.*`)
- Logs and temporary files (`*.log`, `*.tmp`)
- Language-specific patterns

## 8. Repository Categories

### Current Organization

- `apps-android/` - Android/Flutter applications
- `apps-desktop/` - Desktop applications
- `mcp-servers/` - MCP server implementations (ignored for standards)
- `websites/` - Web projects and documentation
- `prompts/` - Shared configuration and standards

### New Repository Placement Rules

- Mobile apps → `apps-android/` or `apps-mobile/`
- Desktop apps → `apps-desktop/`
- Web applications → `apps-web/` or `websites/`
- Libraries/SDKs → `libraries/`
- Tools/Utilities → `tools/`
- Learning/Experiments → `experiments/`

## 9. Automation Support

All standards are supported by:

- `naming-mappings.yaml` - Automatic name conversion
- Renaming scripts (to be created)
- Template generators (to be created)
- Validation scripts (to be created)

## 10. Migration Plan

For existing repositories:

1. Identify current naming inconsistencies
2. Use `naming-mappings.yaml` for automatic conversion
3. Update internal project references
4. Verify build/deployment scripts
5. Update documentation
6. Test thoroughly before finalizing

---

**Note**: These standards apply to all new repositories and should be gradually applied to existing ones during major updates or refactoring.