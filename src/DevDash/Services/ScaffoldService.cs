using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace DevDash.Services;

/// <summary>
/// Service for creating project-level configuration scaffolds from embedded resources.
/// </summary>
public class ScaffoldService : IScaffoldService
{
    private const string ScaffoldPrefix = "workspace_scaffold";

    public bool IsProjectConfigured(string projectPath)
    {
        if (string.IsNullOrWhiteSpace(projectPath) || !Directory.Exists(projectPath))
            return false;

        var claudeDir = Path.Combine(projectPath, ".claude");
        var rulesDir = Path.Combine(claudeDir, "rules");

        return Directory.Exists(rulesDir);
    }

    public async Task<ScaffoldResult> ApplyProjectScaffoldAsync(string projectPath, ProjectScaffoldConfig config)
    {
        if (string.IsNullOrWhiteSpace(projectPath))
            return new ScaffoldResult(0, 0, 0);

        var assembly = Assembly.GetExecutingAssembly();
        var resourceNames = assembly.GetManifestResourceNames()
            .Where(n => n.Contains(ScaffoldPrefix))
            .ToList();

        var filesCreated = 0;
        var filesUpdated = 0;
        var filesSkipped = 0;

        foreach (var resourceName in resourceNames)
        {
            var relativePath = ExtractRelativePath(resourceName);
            if (string.IsNullOrEmpty(relativePath))
            {
                filesSkipped++;
                continue;
            }

            var targetPath = Path.Combine(projectPath, relativePath);
            var content = await ReadEmbeddedResourceAsync(assembly, resourceName);

            if (content == null)
            {
                filesSkipped++;
                continue;
            }

            // Apply template replacements
            content = ApplyTemplateReplacements(content, config, projectPath);

            // Special handling for coding-standards.md
            if (relativePath.EndsWith("coding-standards.md") && !string.IsNullOrEmpty(config.Language))
            {
                var languageStandards = await GetCodingStandardsForLanguageAsync(config.Language);
                if (!string.IsNullOrEmpty(languageStandards))
                {
                    content = content.Replace("{LANGUAGE_SPECIFIC_STANDARDS}", languageStandards);
                }
            }

            // Skip .gitkeep files - just ensure directory exists
            if (Path.GetFileName(relativePath) == ".gitkeep")
            {
                var dir = Path.GetDirectoryName(targetPath);
                if (!string.IsNullOrEmpty(dir))
                    Directory.CreateDirectory(dir);
                filesSkipped++;
                continue;
            }

            var fileExists = File.Exists(targetPath);

            // Ensure directory exists
            var directory = Path.GetDirectoryName(targetPath);
            if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
                Directory.CreateDirectory(directory);

            // Write file
            try
            {
                await File.WriteAllTextAsync(targetPath, content, Encoding.UTF8);
                if (fileExists)
                    filesUpdated++;
                else
                    filesCreated++;
            }
            catch
            {
                filesSkipped++;
            }
        }

        // Create additional directories that are not in scaffold
        await CreateAdditionalStructureAsync(projectPath);

        return new ScaffoldResult(filesCreated, filesUpdated, filesSkipped);
    }

    public async Task<string?> GetCodingStandardsForLanguageAsync(string language)
    {
        var assembly = Assembly.GetExecutingAssembly();
        var resourceName = $"DevDash.rsrc.legacy_workspace_rules.scaffold_rules.coding_standards.{language}.md";

        // Try exact match first
        var content = await ReadEmbeddedResourceAsync(assembly, resourceName);
        if (content != null)
            return content;

        // Try with hyphens converted to underscores
        resourceName = resourceName.Replace("-", "_");
        return await ReadEmbeddedResourceAsync(assembly, resourceName);
    }

    /// <summary>
    /// Creates additional directory structure not included in embedded scaffold.
    /// </summary>
    private async Task CreateAdditionalStructureAsync(string projectPath)
    {
        var directories = new[]
        {
            ".development/specs/implemented",
            ".development/specs/in-progress",
            ".development/specs/planned",
            ".development/specs/backlog",
            ".development/specs/archived",
            ".development/tech-debt",
            ".development/reference/decisions",
            ".development/reference/technical",
            ".development/reference/checklists",
            ".development/archive/completed",
            ".development/archive/analysis",
            ".development/archive/postmortems",
            ".development/archive/legacy",
            ".development/scripts",
            ".personal",
            "docs"
        };

        foreach (var dir in directories)
        {
            var fullPath = Path.Combine(projectPath, dir);
            Directory.CreateDirectory(fullPath);
        }

        // Create README files for key directories
        await CreateReadmeAsync(projectPath, ".development", GetDevelopmentReadme());
        await CreateReadmeAsync(projectPath, ".personal", GetPersonalReadme());
        await CreateReadmeAsync(projectPath, "docs", GetDocsReadme());

        // Create CURRENT-STATUS.md template
        var statusPath = Path.Combine(projectPath, ".development", "CURRENT-STATUS.md");
        if (!File.Exists(statusPath))
        {
            await File.WriteAllTextAsync(statusPath, GetCurrentStatusTemplate(), Encoding.UTF8);
        }
    }

    private async Task CreateReadmeAsync(string projectPath, string directory, string content)
    {
        var readmePath = Path.Combine(projectPath, directory, "README.md");
        if (!File.Exists(readmePath))
        {
            await File.WriteAllTextAsync(readmePath, content, Encoding.UTF8);
        }
    }

    /// <summary>
    /// Applies template variable replacements.
    /// </summary>
    private string ApplyTemplateReplacements(string content, ProjectScaffoldConfig config, string projectPath)
    {
        var projectStructure = GenerateProjectStructure(projectPath);

        var replacements = new Dictionary<string, string>
        {
            { "{PROJECT_NAME}", config.ProjectName },
            { "{PROJECT_DESCRIPTION}", config.ProjectDescription ?? "Project description to be added." },
            { "{TECH_STACK_DESCRIPTION}", config.TechStack ?? "Tech stack to be documented." },
            { "{PROJECT_STRUCTURE}", projectStructure },
            { "{LANGUAGE_SPECIFIC_STANDARDS}", "" } // Will be replaced specifically for coding-standards.md
        };

        foreach (var (placeholder, value) in replacements)
        {
            content = content.Replace(placeholder, value);
        }

        return content;
    }

    /// <summary>
    /// Generates a simple directory tree structure for the project.
    /// </summary>
    private string GenerateProjectStructure(string projectPath)
    {
        try
        {
            var sb = new StringBuilder();
            sb.AppendLine("```");
            sb.AppendLine(Path.GetFileName(projectPath) + "/");

            var directories = Directory.GetDirectories(projectPath)
                .Select(d => Path.GetFileName(d))
                .Where(d => !d.StartsWith(".") && d != "bin" && d != "obj" && d != "node_modules")
                .OrderBy(d => d)
                .Take(10);

            foreach (var dir in directories)
            {
                sb.AppendLine($"├── {dir}/");
            }

            sb.AppendLine("```");
            return sb.ToString();
        }
        catch
        {
            return "```\n[Project structure]\n```";
        }
    }

    /// <summary>
    /// Extracts the relative file path from an embedded resource name.
    /// </summary>
    private static string? ExtractRelativePath(string resourceName)
    {
        var prefixIndex = resourceName.IndexOf(ScaffoldPrefix, StringComparison.Ordinal);
        if (prefixIndex < 0)
            return null;

        var afterPrefix = resourceName[(prefixIndex + ScaffoldPrefix.Length)..];
        if (afterPrefix.StartsWith("."))
            afterPrefix = afterPrefix[1..];

        if (string.IsNullOrEmpty(afterPrefix))
            return null;

        var result = new List<string>();
        var current = afterPrefix;

        while (!string.IsNullOrEmpty(current))
        {
            if (current.StartsWith("."))
            {
                var nextDot = current.IndexOf('.', 1);
                if (nextDot < 0)
                {
                    result.Add(RestoreOriginalName(current));
                    break;
                }
                else
                {
                    result.Add(RestoreOriginalName(current[..nextDot]));
                    current = current[(nextDot + 1)..];
                }
            }
            else
            {
                var nextDot = current.IndexOf('.');
                if (nextDot < 0)
                {
                    result.Add(RestoreOriginalName(current));
                    break;
                }
                else
                {
                    result.Add(RestoreOriginalName(current[..nextDot]));
                    current = current[(nextDot + 1)..];
                }
            }
        }

        if (result.Count < 2)
            return null;

        string filename;
        List<string> dirParts;

        if (result[^1].StartsWith("."))
        {
            filename = result[^1];
            dirParts = result.Take(result.Count - 1).ToList();
        }
        else
        {
            var extension = result[^1];
            var filenameBase = result[^2];
            filename = filenameBase + "." + extension;
            dirParts = result.Take(result.Count - 2).ToList();
        }

        if (dirParts.Count == 0)
            return filename;

        var directoryPath = string.Join(Path.DirectorySeparatorChar.ToString(), dirParts);
        return Path.Combine(directoryPath, filename);
    }

    private static string RestoreOriginalName(string segment)
    {
        return segment.Replace("_", "-");
    }

    private static async Task<string?> ReadEmbeddedResourceAsync(Assembly assembly, string resourceName)
    {
        try
        {
            using var stream = assembly.GetManifestResourceStream(resourceName);
            if (stream == null)
                return null;

            using var reader = new StreamReader(stream, Encoding.UTF8);
            return await reader.ReadToEndAsync();
        }
        catch
        {
            return null;
        }
    }

    // Template content methods
    private string GetDevelopmentReadme() => @"# .development/ - Development Documentation

Private development documentation for spec-driven development workflow.

**Committed to git** - shared development documentation.

## Quick Start

1. Read `CURRENT-STATUS.md` for project state
2. Check `specs/` for feature specifications
3. Check `tech-debt/` for active issues

## Structure

```
.development/
├── CURRENT-STATUS.md      # Current project state
├── specs/                 # Feature specifications
│   ├── implemented/       # Completed features
│   ├── in-progress/       # Currently being developed
│   ├── planned/           # Confirmed for next releases
│   ├── backlog/           # Validated but not scheduled
│   └── archived/          # Deprecated/cancelled
├── tech-debt/             # Active technical debt
├── reference/             # Reference documentation
│   ├── decisions/         # Architecture Decision Records (ADR)
│   ├── technical/         # Technical notes
│   └── checklists/        # Workflow checklists
├── archive/               # Historical
│   ├── completed/         # Resolved issues
│   ├── analysis/          # Agent reports
│   ├── postmortems/       # Post-mortems
│   └── legacy/            # Old files
└── scripts/               # Utility scripts
```

## Related

- Public docs: `docs/`
- Personal notes: `.personal/`
";

    private string GetPersonalReadme() => @"# .personal/ - Personal Notes

Private notes and work-in-progress. **Not tracked in git.**

## Usage

This directory is for your personal notes, analysis, and work-in-progress that shouldn't be committed.

Suggested structure:
- `analysis/` - Your analysis and research
- `strategy/` - Strategic planning
- `work-in-progress/` - Current work notes
- `completed/` - Finished work
- `ideas/` - Ideas and brainstorming
- `scripts/` - Personal utility scripts
";

    private string GetDocsReadme() => @"# Documentation

Public project documentation.

## Contents

Add project documentation here:
- Architecture diagrams
- API documentation
- User guides
- Technical specifications
";

    private string GetCurrentStatusTemplate() => @"# Current Status

## Project State

**Last Updated**: [DATE]

**Current Phase**: [Development/Alpha/Beta/Production]

**Active Work**: [Description of current focus]

## Recent Milestones

- [Milestone 1] - [Date]
- [Milestone 2] - [Date]

## Next Steps

- [ ] Task 1
- [ ] Task 2
- [ ] Task 3

## Active Issues

See `.development/tech-debt/` for tracked technical debt.

## Notes

[Any important notes or context]
";
}
