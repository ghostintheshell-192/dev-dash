namespace DevDash.Models;

/// <summary>
/// Rappresenta un workspace (contenitore di progetti).
/// Esempi: Coding (/data/repos), Writing (Vault@Racconti)
/// </summary>
public record Workspace
{
    public required int Id { get; init; }
    public required string Name { get; init; }
    public required string Path { get; init; }
    public required string Type { get; init; }  // "coding" | "writing"
    public required string Icon { get; init; }  // Emoji

    public static Workspace Coding => new()
    {
        Id = 1,
        Name = "Coding",
        Path = "/data/repos",
        Type = "coding",
        Icon = "💻"
    };

    public static Workspace Writing => new()
    {
        Id = 2,
        Name = "Writing",
        Path = "/data/documenti/Vault@Racconti",
        Type = "writing",
        Icon = "✍️"
    };
}
