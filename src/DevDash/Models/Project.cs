namespace DevDash.Models;

/// <summary>
/// Rappresenta un progetto all'interno di un workspace.
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
}
