namespace DevDash.Models;

/// <summary>
/// Application settings stored in a JSON configuration file.
/// </summary>
public class AppSettings
{
    /// <summary>
    /// Path to the workspace directory containing projects.
    /// </summary>
    public string? WorkspacePath { get; set; }

    /// <summary>
    /// Path to the Claude configuration directory (typically ~/.claude).
    /// </summary>
    public string? ClaudeConfigPath { get; set; }
}
