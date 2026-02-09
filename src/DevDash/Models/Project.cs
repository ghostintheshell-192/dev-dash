namespace DevDash.Models;

/// <summary>
/// Represents a project within a workspace.
/// Each project has its own .claude/ configuration.
/// </summary>
public record Project
{
    public required string Id { get; init; }
    public required string Name { get; init; }
    public required string Path { get; init; }
    public string? Branch { get; init; }
    public bool HasPersonal { get; init; }
    public bool HasDocs { get; init; }
    public string? Language { get; init; }  // "csharp", "python", "flutter", etc.
    public int WorkspaceId { get; init; }

    // Claude configuration status
    public bool IsConfigured { get; init; }  // Has .claude/ directory with rules/
    public bool HasClaudeDir { get; init; }  // Has .claude/ directory
    public bool HasRules { get; init; }      // Has .claude/rules/ directory
}
