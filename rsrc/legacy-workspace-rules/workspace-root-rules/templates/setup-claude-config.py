#!/usr/bin/env python3
"""
Setup Claude Code configuration for a new project.

Creates self-contained Claude configuration:
- .claude/settings.json (permissions + hooks)
- .claude/commands/handoff.md
- .claude/skills/session-handoff/
- .development/scripts/session-archive.py
- .memory-bank/ structure

Usage:
    python3 setup-claude-config.py /path/to/project
    python3 setup-claude-config.py .  # current directory
"""

import argparse
import json
import shutil
import sys
from pathlib import Path

TEMPLATE_DIR = Path(__file__).parent


def setup_claude_folder(project_path: Path) -> None:
    """Copy .claude/ templates to project."""
    claude_dir = project_path / ".claude"
    claude_template = TEMPLATE_DIR / "claude"

    # Create directories
    (claude_dir / "commands").mkdir(parents=True, exist_ok=True)
    (claude_dir / "skills" / "session-handoff").mkdir(parents=True, exist_ok=True)

    # Copy settings.json (merge if exists)
    settings_file = claude_dir / "settings.json"
    template_settings = claude_template / "settings.json"

    if settings_file.exists():
        print(f"  [INFO] .claude/settings.json exists, merging permissions...")
        with open(settings_file) as f:
            existing = json.load(f)
        with open(template_settings) as f:
            template = json.load(f)

        # Merge permissions
        if "permissions" not in existing:
            existing["permissions"] = template["permissions"]
            print(f"  [OK] Added permissions to settings.json")
        else:
            print(f"  [SKIP] Permissions already exist")

        # Merge SessionEnd hook if not present
        if "hooks" not in existing:
            existing["hooks"] = {}
        if "SessionEnd" not in existing["hooks"]:
            existing["hooks"]["SessionEnd"] = template["hooks"]["SessionEnd"]
            print(f"  [OK] Added SessionEnd hook")

        with open(settings_file, "w") as f:
            json.dump(existing, f, indent=2)
    else:
        shutil.copy(template_settings, settings_file)
        print(f"  [OK] Created .claude/settings.json")

    # Copy command
    cmd_src = claude_template / "commands" / "handoff.md"
    cmd_dst = claude_dir / "commands" / "handoff.md"
    if not cmd_dst.exists():
        shutil.copy(cmd_src, cmd_dst)
        print(f"  [OK] Created .claude/commands/handoff.md")
    else:
        print(f"  [SKIP] handoff command already exists")

    # Copy skill
    skill_src = claude_template / "skills" / "session-handoff" / "SKILL.md"
    skill_dst = claude_dir / "skills" / "session-handoff" / "SKILL.md"
    if not skill_dst.exists():
        shutil.copy(skill_src, skill_dst)
        print(f"  [OK] Created .claude/skills/session-handoff/")
    else:
        print(f"  [SKIP] session-handoff skill already exists")


def setup_development_scripts(project_path: Path) -> None:
    """Copy development scripts to project."""
    scripts_dir = project_path / ".development" / "scripts"
    scripts_template = TEMPLATE_DIR / "development" / "scripts"

    scripts_dir.mkdir(parents=True, exist_ok=True)

    # Copy session-archive.py
    archive_src = scripts_template / "session-archive.py"
    archive_dst = scripts_dir / "session-archive.py"

    if not archive_dst.exists():
        shutil.copy(archive_src, archive_dst)
        archive_dst.chmod(0o755)
        print(f"  [OK] Created .development/scripts/session-archive.py")
    else:
        print(f"  [SKIP] session-archive.py already exists")


def setup_memory_bank(project_path: Path) -> None:
    """Create .memory-bank/ structure."""
    memory_dir = project_path / ".memory-bank"
    sessions_dir = memory_dir / "sessions"

    sessions_dir.mkdir(parents=True, exist_ok=True)
    print(f"  [OK] Created .memory-bank/sessions/")

    # Add .gitignore
    gitignore = memory_dir / ".gitignore"
    if not gitignore.exists():
        gitignore.write_text("# Session transcripts (backups only)\nsessions/\n")
        print(f"  [OK] Created .memory-bank/.gitignore")


def main():
    parser = argparse.ArgumentParser(
        description="Setup Claude Code configuration for a project"
    )
    parser.add_argument(
        "project_path",
        type=str,
        help="Path to the project directory"
    )

    args = parser.parse_args()
    project_path = Path(args.project_path).resolve()

    if not project_path.exists():
        print(f"Error: Project path does not exist: {project_path}")
        sys.exit(1)

    if not project_path.is_dir():
        print(f"Error: Project path is not a directory: {project_path}")
        sys.exit(1)

    print(f"Setting up Claude Code config for: {project_path.name}")
    print()

    # Setup steps
    setup_claude_folder(project_path)
    setup_development_scripts(project_path)
    setup_memory_bank(project_path)

    print()
    print("✅ Done! Claude Code configuration is now self-contained.")
    print()
    print("Next steps:")
    print("  1. Update .claude/settings.json with project-specific hooks")
    print("  2. Test handoff: in Claude session, say 'ciao' or '/handoff'")
    print("  3. Session transcripts will be saved to .memory-bank/sessions/")


if __name__ == "__main__":
    main()
