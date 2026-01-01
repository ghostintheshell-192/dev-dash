namespace DevDash.Models;

/// <summary>
/// Rappresenta un workspace (contenitore di progetti).
/// Un workspace ha una struttura standard:
/// - .rules/           → Standards e bootstrap
/// - .memory-bank/     → Memoria operativa (handoff sessioni)
/// - CLAUDE.md         → Entry point per Claude Code
/// </summary>
public record Workspace
{
    public required int Id { get; init; }
    public required string Name { get; init; }
    public required string Path { get; init; }
    public required string Type { get; init; }  // "coding" | "writing"
    public required string Icon { get; init; }

    // Struttura workspace
    public bool HasRules { get; init; }         // .rules/ presente
    public bool HasMemoryBank { get; init; }    // .memory-bank/ presente
    public bool HasClaudeMd { get; init; }      // CLAUDE.md presente
    public string? BootstrapType { get; init; } // "coding" | "writing" | null (da bootstrap-*.md)
}
