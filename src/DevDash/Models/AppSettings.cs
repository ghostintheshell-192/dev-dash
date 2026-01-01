namespace DevDash.Models;

public record AppSettings
{
    public string? WorkspacePath { get; init; }
    public string? ClaudeConfigPath { get; init; }
    public string WorkspaceType { get; init; } = "coding";  // "coding" | "writing"
}
