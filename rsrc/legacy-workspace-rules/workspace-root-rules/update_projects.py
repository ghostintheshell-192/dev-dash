#!/usr/bin/env python3
"""
Script semplice per generare projects.yaml con la mappatura automatica dei progetti.
"""

import os
import yaml
from pathlib import Path

def detect_project_type(project_path):
    """Rileva il tipo di progetto basandosi sui file presenti."""
    files = []
    for root, dirs, filenames in os.walk(project_path):
        # Limita la ricerca ai primi 2 livelli per performance
        level = root.replace(project_path, '').count(os.sep)
        if level < 2:
            files.extend([f for f in filenames if not f.startswith('.')])
    
    files_set = set(files)
    
    # Controlla i pattern dal goto.yaml
    if 'pubspec.yaml' in files_set or any(f.endswith('.dart') for f in files):
        return 'flutter'
    elif any(f.endswith(('.csproj', '.sln', '.cs')) for f in files) or 'appsettings.json' in files_set:
        return 'csharp'
    elif files_set & {'requirements.txt', 'pyproject.toml', 'setup.py', 'Pipfile'} or any(f.endswith('.py') for f in files):
        return 'python'
    elif 'package.json' in files_set or any(f.endswith(('.js', '.ts', '.tsx', '.jsx')) for f in files) or 'tsconfig.json' in files_set:
        return 'javascript_typescript'
    elif files_set & {'CMakeLists.txt', 'Makefile'} or any(f.endswith(('.cpp', '.hpp', '.h', '.c')) for f in files):
        return 'cpp'
    else:
        return 'unknown'

def scan_projects(base_path=None):
    """Scansiona tutti i progetti nella directory base e genera la mappatura."""
    if base_path is None:
        base_path = Path.cwd().parent  # Assumes script is in rules/ subdirectory
    else:
        base_path = Path(base_path)
    repos_path = base_path
    projects = {}

    # Directory da escludere (non sono progetti)
    excluded_dirs = {
        'rules', 'prompts', '.git', '__pycache__', 'node_modules',
        '.vscode', '.idea', 'archive', 'conversations', '.claude',
        '.npm-global'
    }

    for item in repos_path.iterdir():
        if item.is_dir() and item.name not in excluded_dirs:
            # Con flat structure, i progetti sono direttamente al primo livello
            # Non scansionare sottodirectory (src/, tests/, etc.)
            project_type = detect_project_type(str(item))
            if project_type != 'unknown':
                projects[item.name] = {
                    'type': project_type,
                    'path': str(item.relative_to(repos_path))
                }

    return projects

def generate_projects_yaml():
    """Genera il file projects.yaml."""
    projects = scan_projects()
    
    output = {
        'projects': projects,
        'last_updated': '# Generato automaticamente - non modificare manualmente'
    }
    
    with open('projects.yaml', 'w') as f:
        f.write(output['last_updated'] + '\n')
        yaml.dump({'projects': projects}, f, default_flow_style=False, sort_keys=True)
    
    print(f"Generato projects.yaml con {len(projects)} progetti")

if __name__ == '__main__':
    generate_projects_yaml()