#!/usr/bin/env python3
"""
Setup project documentation structure.

Creates:
- .personal/ folder structure with templates
- .claude/settings.json with SessionStart hook
- Adds .personal/ to .gitignore if not present

Usage:
    python3 setup-project-docs.py /path/to/project
    python3 setup-project-docs.py .  # current directory
"""

import argparse
import json
import shutil
import sys
from pathlib import Path

TEMPLATE_DIR = Path(__file__).parent / ".personal"


def setup_personal_folder(project_path: Path) -> None:
    """Copy .personal/ template to project."""
    personal_dir = project_path / ".personal"

    if personal_dir.exists():
        print(f"  [SKIP] .personal/ already exists")
        return

    shutil.copytree(TEMPLATE_DIR, personal_dir)
    print(f"  [OK] Created .personal/ structure")


def setup_claude_hook(project_path: Path) -> None:
    """Create or update .claude/settings.json with SessionStart hook."""
    claude_dir = project_path / ".claude"
    settings_file = claude_dir / "settings.json"

    hook_config = {
        "hooks": {
            "SessionStart": [
                {
                    "matcher": "startup",
                    "hooks": [
                        {
                            "type": "command",
                            "command": f"python3 {project_path}/.personal/scripts/generate-index.py 2>/dev/null || true"
                        }
                    ]
                }
            ]
        }
    }

    if settings_file.exists():
        # Merge with existing settings
        try:
            with open(settings_file) as f:
                existing = json.load(f)

            if "hooks" not in existing:
                existing["hooks"] = {}
            if "SessionStart" not in existing["hooks"]:
                existing["hooks"]["SessionStart"] = hook_config["hooks"]["SessionStart"]
                with open(settings_file, "w") as f:
                    json.dump(existing, f, indent=2)
                print(f"  [OK] Added SessionStart hook to existing settings.json")
            else:
                print(f"  [SKIP] SessionStart hook already exists")
        except json.JSONDecodeError:
            print(f"  [WARN] Could not parse existing settings.json, skipping hook setup")
    else:
        claude_dir.mkdir(exist_ok=True)
        with open(settings_file, "w") as f:
            json.dump(hook_config, f, indent=2)
        print(f"  [OK] Created .claude/settings.json with SessionStart hook")


def update_gitignore(project_path: Path) -> None:
    """Add .personal/ to .gitignore if not present."""
    gitignore = project_path / ".gitignore"

    entries_to_add = [".personal/"]

    existing_content = ""
    if gitignore.exists():
        existing_content = gitignore.read_text()

    lines_to_add = []
    for entry in entries_to_add:
        if entry not in existing_content:
            lines_to_add.append(entry)

    if lines_to_add:
        with open(gitignore, "a") as f:
            if existing_content and not existing_content.endswith("\n"):
                f.write("\n")
            f.write("\n# Private documentation (not committed)\n")
            for line in lines_to_add:
                f.write(f"{line}\n")
        print(f"  [OK] Added {', '.join(lines_to_add)} to .gitignore")
    else:
        print(f"  [SKIP] .gitignore already contains .personal/")


def run_index_generator(project_path: Path) -> None:
    """Run the index generator to create initial INDEX.md."""
    import subprocess

    script = project_path / ".personal" / "scripts" / "generate-index.py"
    if script.exists():
        result = subprocess.run(
            ["python3", str(script)],
            capture_output=True,
            text=True
        )
        if result.returncode == 0:
            print(f"  [OK] Generated initial INDEX.md")
        else:
            print(f"  [WARN] Failed to generate INDEX.md: {result.stderr}")
    else:
        print(f"  [SKIP] generate-index.py not found")


def main():
    parser = argparse.ArgumentParser(
        description="Setup project documentation structure"
    )
    parser.add_argument(
        "project_path",
        type=str,
        help="Path to the project directory"
    )
    parser.add_argument(
        "--no-hook",
        action="store_true",
        help="Skip creating SessionStart hook"
    )
    parser.add_argument(
        "--no-gitignore",
        action="store_true",
        help="Skip updating .gitignore"
    )

    args = parser.parse_args()

    project_path = Path(args.project_path).resolve()

    if not project_path.exists():
        print(f"Error: Project path does not exist: {project_path}")
        sys.exit(1)

    if not project_path.is_dir():
        print(f"Error: Project path is not a directory: {project_path}")
        sys.exit(1)

    print(f"Setting up documentation for: {project_path}")
    print()

    # Setup steps
    setup_personal_folder(project_path)

    if not args.no_hook:
        setup_claude_hook(project_path)

    if not args.no_gitignore:
        update_gitignore(project_path)

    run_index_generator(project_path)

    print()
    print("Done! Next steps:")
    print("  1. Edit .personal/CURRENT-STATUS.md with project info")
    print("  2. Start a new Claude session to test the hook")
    print("  3. Check .personal/INDEX.md was generated")


if __name__ == "__main__":
    main()
