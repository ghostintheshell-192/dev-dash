using System.Collections.Generic;

namespace DevDash.Models;

/// <summary>
/// Represents a configured workspace with its settings.
/// </summary>
public record WorkspaceConfig
{
    public string Path { get; init; } = string.Empty;
    public string Name { get; init; } = string.Empty;
    public string Type { get; init; } = "coding";  // "coding" | "writing"
}

public record AppSettings
{
    /// <summary>
    /// List of configured workspace paths.
    /// </summary>
    public List<WorkspaceConfig> Workspaces { get; init; } = [];

    /// <summary>
    /// Index of the currently selected workspace.
    /// </summary>
    public int SelectedWorkspaceIndex { get; init; } = 0;

    /// <summary>
    /// Path to Claude Code global configuration (~/.claude).
    /// </summary>
    public string? ClaudeConfigPath { get; init; }

    // Legacy properties for backwards compatibility
    [System.Text.Json.Serialization.JsonIgnore]
    public string? WorkspacePath => Workspaces.Count > 0 ? Workspaces[SelectedWorkspaceIndex >= 0 && SelectedWorkspaceIndex < Workspaces.Count ? SelectedWorkspaceIndex : 0].Path : null;

    [System.Text.Json.Serialization.JsonIgnore]
    public string WorkspaceType => Workspaces.Count > 0 ? Workspaces[SelectedWorkspaceIndex >= 0 && SelectedWorkspaceIndex < Workspaces.Count ? SelectedWorkspaceIndex : 0].Type : "coding";
}
