using DevDash.Models;

namespace DevDash.ViewModels;

/// <summary>
/// Represents a tech debt item from .development/tech-debt/
/// </summary>
public class TechDebtItemViewModel
{
    public required string FileName { get; init; }
    public required string FullPath { get; init; }
    public string? Priority { get; init; }
    public string? Status { get; init; }
    public FrontmatterData? Frontmatter { get; init; }
}
