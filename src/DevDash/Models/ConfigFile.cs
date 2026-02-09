namespace DevDash.Models;

/// <summary>
/// Rappresenta un file di configurazione Claude Code
/// </summary>
public record ConfigFile
{
    public required string Name { get; init; }
    public required string Path { get; init; }
    public required ConfigLevel Level { get; init; }
    public string? IconName { get; init; }
}

/// <summary>
/// Livello di configurazione Claude Code
/// </summary>
public enum ConfigLevel
{
    Global,     // ~/.claude/
    Workspace,  // /data/repos/
    Project     // singolo progetto
}
