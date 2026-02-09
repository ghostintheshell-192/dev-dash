using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using DevDash.Models;

namespace DevDash.Services;

public class WorkspaceService : IWorkspaceService
{
    private readonly IFileSystemService _fileSystem;
    private readonly IAppSettingsService _appSettings;

    public WorkspaceService(IFileSystemService fileSystem, IAppSettingsService appSettings)
    {
        _fileSystem = fileSystem;
        _appSettings = appSettings;
    }

    public IReadOnlyList<Workspace> GetWorkspaces()
    {
        var settings = _appSettings.Load();
        var workspacePath = settings.WorkspacePath;

        if (string.IsNullOrWhiteSpace(workspacePath) || !_fileSystem.DirectoryExists(workspacePath))
        {
            return new List<Workspace>().AsReadOnly();
        }

        // Rileva il tipo di workspace dalla struttura
        var hasRules = _fileSystem.DirectoryExists(Path.Combine(workspacePath, ".rules"));
        var hasMemoryBank = _fileSystem.DirectoryExists(Path.Combine(workspacePath, ".memory-bank"));
        var hasClaudeMd = _fileSystem.FileExists(Path.Combine(workspacePath, "CLAUDE.md"));
        var bootstrapType = DetectBootstrapType(workspacePath);

        // Determina tipo e icona
        var type = bootstrapType ?? "coding";
        var icon = type == "writing" ? "✍️" : "💻";
        var name = Path.GetFileName(workspacePath);

        return new List<Workspace>
        {
            new()
            {
                Id = 1,
                Name = name,
                Path = workspacePath,
                Type = type,
                Icon = icon
            }
        }.AsReadOnly();
    }

    private string? DetectBootstrapType(string workspacePath)
    {
        var rulesPath = Path.Combine(workspacePath, ".rules");
        if (!_fileSystem.DirectoryExists(rulesPath))
            return null;

        // Cerca bootstrap-coding.md o bootstrap-writing.md
        if (_fileSystem.FileExists(Path.Combine(rulesPath, "bootstrap-coding.md")))
            return "coding";
        if (_fileSystem.FileExists(Path.Combine(rulesPath, "bootstrap-writing.md")))
            return "writing";

        return null;
    }

    public Task<IReadOnlyList<Project>> GetProjectsAsync(Workspace workspace)
    {
        var projects = new List<Project>();

        if (!_fileSystem.DirectoryExists(workspace.Path))
            return Task.FromResult<IReadOnlyList<Project>>(projects.AsReadOnly());

        var directories = _fileSystem.GetDirectories(workspace.Path);
        int idCounter = 0;

        foreach (var dir in directories)
        {
            var name = Path.GetFileName(dir);

            // Salta directory speciali e nascoste
            if (name.StartsWith('.') || name == "node_modules" || name == "bin" || name == "obj")
                continue;

            var hasPersonal = _fileSystem.DirectoryExists(Path.Combine(dir, ".personal"));
            var hasDocs = _fileSystem.DirectoryExists(Path.Combine(dir, "docs"));
            var hasClaudeMd = _fileSystem.FileExists(Path.Combine(dir, "CLAUDE.md"));

            // Check Claude Code configuration status
            var claudeDir = Path.Combine(dir, ".claude");
            var rulesDir = Path.Combine(claudeDir, "rules");
            var hasClaudeDir = _fileSystem.DirectoryExists(claudeDir);
            var hasRules = _fileSystem.DirectoryExists(rulesDir);
            var isConfigured = hasRules; // Project is configured if it has .claude/rules/

            projects.Add(new Project
            {
                Id = $"{workspace.Id}_{idCounter++}",
                Name = name,
                Path = dir,
                Branch = GetCurrentBranch(dir),
                HasPersonal = hasPersonal,
                HasDocs = hasDocs,
                Language = DetectLanguage(dir),
                WorkspaceId = workspace.Id,
                IsConfigured = isConfigured,
                HasClaudeDir = hasClaudeDir,
                HasRules = hasRules
            });
        }

        // Ordina: prima quelli con .personal, poi alfabetico
        var sorted = projects
            .OrderByDescending(p => p.HasPersonal)
            .ThenBy(p => p.Name)
            .ToList();

        return Task.FromResult<IReadOnlyList<Project>>(sorted.AsReadOnly());
    }

    public string? DetectLanguage(string projectPath)
    {
        // Rileva linguaggio in base ai file presenti (basato su goto.yaml)
        if (_fileSystem.GetFiles(projectPath, "*.csproj").Any() ||
            _fileSystem.GetFiles(projectPath, "*.sln").Any())
            return "csharp";

        if (_fileSystem.FileExists(Path.Combine(projectPath, "pubspec.yaml")))
            return "flutter";

        if (_fileSystem.FileExists(Path.Combine(projectPath, "requirements.txt")) ||
            _fileSystem.GetFiles(projectPath, "*.py").Any())
            return "python";

        if (_fileSystem.FileExists(Path.Combine(projectPath, "package.json")))
        {
            if (_fileSystem.FileExists(Path.Combine(projectPath, "tsconfig.json")))
                return "typescript";
            return "javascript";
        }

        if (_fileSystem.FileExists(Path.Combine(projectPath, "CMakeLists.txt")) ||
            _fileSystem.GetFiles(projectPath, "*.cpp").Any())
            return "cpp";

        if (_fileSystem.FileExists(Path.Combine(projectPath, "go.mod")))
            return "go";

        if (_fileSystem.FileExists(Path.Combine(projectPath, "Cargo.toml")))
            return "rust";

        return null;
    }

    public string? GetCurrentBranch(string projectPath)
    {
        var gitDir = Path.Combine(projectPath, ".git");
        if (!_fileSystem.DirectoryExists(gitDir))
            return null;

        try
        {
            var headFile = Path.Combine(gitDir, "HEAD");
            if (!_fileSystem.FileExists(headFile))
                return null;

            var headContent = File.ReadAllText(headFile).Trim();

            // Format: "ref: refs/heads/branch-name"
            if (headContent.StartsWith("ref: refs/heads/"))
                return headContent["ref: refs/heads/".Length..];

            // Detached HEAD - restituisci hash abbreviato
            if (headContent.Length >= 7)
                return headContent[..7];
        }
        catch
        {
            // Ignora errori
        }

        return null;
    }
}
