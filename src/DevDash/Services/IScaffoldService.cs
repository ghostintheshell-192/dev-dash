using System.Collections.Generic;
using System.Threading.Tasks;

namespace DevDash.Services;

/// <summary>
/// Result of applying a scaffold to a project.
/// </summary>
public record ScaffoldResult(
    int FilesCreated,
    int FilesUpdated,
    int FilesSkipped
);

/// <summary>
/// Configuration for initializing a project with Claude Code scaffold.
/// </summary>
public record ProjectScaffoldConfig
{
    public required string ProjectName { get; init; }
    public string? ProjectDescription { get; init; }
    public string? TechStack { get; init; }
    public string? Language { get; init; }  // "csharp", "python", "javascript", "flutter", "c-cpp"
};

/// <summary>
/// Service for creating project-level Claude Code configuration scaffolds.
/// </summary>
public interface IScaffoldService
{
    /// <summary>
    /// Applies the project scaffold to the specified path.
    /// Creates Claude Code configuration files (.claude/, .memory-bank/, .development/).
    /// </summary>
    /// <param name="projectPath">The project root path.</param>
    /// <param name="config">Configuration for customizing the scaffold.</param>
    /// <returns>Result containing counts of files created, updated, and skipped.</returns>
    Task<ScaffoldResult> ApplyProjectScaffoldAsync(string projectPath, ProjectScaffoldConfig config);

    /// <summary>
    /// Checks if a project has already been configured with Claude Code files.
    /// </summary>
    /// <param name="projectPath">The project root path.</param>
    /// <returns>True if the project has .claude/rules/ directory.</returns>
    bool IsProjectConfigured(string projectPath);

    /// <summary>
    /// Gets the language-specific coding standards content for a given language.
    /// </summary>
    /// <param name="language">Language identifier (csharp, python, javascript, flutter, c-cpp).</param>
    /// <returns>Markdown content for coding standards, or null if language not found.</returns>
    Task<string?> GetCodingStandardsForLanguageAsync(string language);
}
