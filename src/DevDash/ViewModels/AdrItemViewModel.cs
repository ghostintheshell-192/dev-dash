using DevDash.Models;

namespace DevDash.ViewModels;

/// <summary>
/// Represents an ADR from .development/reference/decisions/
/// </summary>
public class AdrItemViewModel
{
    public required string Number { get; init; }
    public required string Title { get; init; }
    public required string FullPath { get; init; }
    public string DisplayName => $"ADR-{Number}: {Title}";
    public FrontmatterData? Frontmatter { get; init; }
}
