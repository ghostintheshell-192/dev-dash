# VSCode IDE Standards

## Overview
Standard VSCode configuration for consistent development experience across projects.

## Required Files
Create `.vscode/` directory with these files (commit to repository):

### **settings.json** - Workspace Settings
```json
{
    "python.defaultInterpreterPath": "./venv/bin/python",
    "python.terminal.activateEnvironment": true,
    "python.formatting.provider": "black",
    "python.linting.enabled": true,
    "python.linting.flake8Enabled": true,
    "editor.formatOnSave": true,
    "editor.codeActionsOnSave": {
        "source.organizeImports": true
    },
    "files.exclude": {
        "**/__pycache__": true,
        "**/venv": true,
        "**/.pytest_cache": true,
        "**/.mypy_cache": true
    }
}
```

### **extensions.json** - Recommended Extensions
```json
{
    "recommendations": [
        "ms-python.python",
        "ms-python.black-formatter",
        "ms-python.flake8",
        "ms-python.mypy-type-checker",
        "ms-vscode.vscode-json",
        "redhat.vscode-yaml",
        "ms-python.pytest"
    ]
}
```

### **launch.json** - Debug Configuration
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Python: Current File",
            "type": "python",
            "request": "launch",
            "program": "${file}",
            "console": "integratedTerminal",
            "cwd": "${workspaceFolder}"
        },
        {
            "name": "Python: Main Module",
            "type": "python",
            "request": "launch",
            "module": "project_name.main",
            "console": "integratedTerminal",
            "cwd": "${workspaceFolder}"
        },
        {
            "name": "Python: Run Script",
            "type": "python",
            "request": "launch",
            "program": "${workspaceFolder}/run.py",
            "console": "integratedTerminal",
            "cwd": "${workspaceFolder}"
        }
    ]
}
```

### **tasks.json** - Task Automation
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Install Dependencies",
            "type": "shell",
            "command": "pip",
            "args": ["install", "-r", "requirements.txt"],
            "group": "build",
            "presentation": {
                "echo": true,
                "reveal": "always"
            }
        },
        {
            "label": "Run Tests",
            "type": "shell",
            "command": "pytest",
            "group": "test",
            "presentation": {
                "echo": true,
                "reveal": "always"
            }
        },
        {
            "label": "Format Code",
            "type": "shell",
            "command": "black",
            "args": ["src/"],
            "group": "build"
        },
        {
            "label": "Type Check",
            "type": "shell",
            "command": "mypy",
            "args": ["src/"],
            "group": "test"
        }
    ]
}
```

## .gitignore Considerations

### Keep These VSCode Files
```gitignore
# VSCode workspace settings (commit these)
!.vscode/settings.json
!.vscode/extensions.json
!.vscode/launch.json
!.vscode/tasks.json
```

### Ignore These VSCode Files
```gitignore
# VSCode user-specific files (ignore these)
.vscode/c_cpp_properties.json
.vscode/launch.test.json
.vscode/.ropeproject
.vscode/ltex.dictionary.*.txt
```

## Language-Specific Configurations

### Python Projects
- Automatic virtual environment detection
- Black formatting on save
- Flake8 linting enabled
- Import organization on save
- Pytest integration

### C# Projects (Future)
- OmniSharp integration
- .NET debugging
- NuGet package management

### Flutter Projects (Future)
- Dart/Flutter extensions
- Hot reload support
- Widget inspector

## Setup Instructions

1. **Open project in VSCode**
2. **Install recommended extensions** (prompt will appear)
3. **Select Python interpreter**: `Ctrl+Shift+P` → "Python: Select Interpreter" → choose `./venv/bin/python`
4. **Verify setup**: Extensions should activate automatically

## Keyboard Shortcuts
- `F5` - Start debugging
- `Ctrl+Shift+P` - Command palette
- `Ctrl+Shift+T` - Run task
- `Ctrl+Shift+\`` - Open terminal