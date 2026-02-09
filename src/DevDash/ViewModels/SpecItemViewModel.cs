using DevDash.Models;

namespace DevDash.ViewModels;

/// <summary>
/// Represents a spec from .development/specs/
/// </summary>
public class SpecItemViewModel
{
    public required string FileName { get; init; }
    public required string FullPath { get; init; }
    public required string Group { get; init; }
    public string? Priority { get; init; }
    public string? Status { get; init; }
    public FrontmatterData? Frontmatter { get; init; }
}
