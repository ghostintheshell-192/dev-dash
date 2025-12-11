using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using DevDash.Models;

namespace DevDash.Services;

public class WorkspaceService : IWorkspaceService
{
    private readonly IFileSystemService _fileSystem;

    // Workspace predefiniti (in futuro letti da configurazione)
    private readonly List<Workspace> _workspaces =
    [
        Workspace.Coding,
        Workspace.Writing
    ];

    public WorkspaceService(IFileSystemService fileSystem)
    {
        _fileSystem = fileSystem;
    }

    public IReadOnlyList<Workspace> GetWorkspaces() => _workspaces.AsReadOnly();

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

            // Salta directory speciali
            if (name.StartsWith('.') || name == "rules" || name == "node_modules")
                continue;

            var hasPersonal = _fileSystem.DirectoryExists(Path.Combine(dir, ".personal"));
            var hasDocs = _fileSystem.DirectoryExists(Path.Combine(dir, "docs"));

            projects.Add(new Project
            {
                Id = $"{workspace.Id}_{idCounter++}",
                Name = name,
                Path = dir,
                Branch = GetCurrentBranch(dir),
                HasPersonal = hasPersonal,
                HasDocs = hasDocs,
                Language = DetectLanguage(dir),
                WorkspaceId = workspace.Id
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
            // Potrebbe essere TypeScript o JavaScript
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
