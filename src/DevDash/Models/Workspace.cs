namespace DevDash.Models;

/// <summary>
/// Represents a workspace (container of projects).
/// Workspaces are directories containing multiple projects.
/// Each project manages its own .claude/ configuration independently.
/// </summary>
public record Workspace
{
    public required int Id { get; init; }
    public required string Name { get; init; }
    public required string Path { get; init; }
    public required string Type { get; init; }  // "coding" | "writing"
    public required string Icon { get; init; }
}
