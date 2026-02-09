namespace DevDash.Models;

/// <summary>
/// Represents the currently active tab in the main content area.
/// </summary>
public enum ActiveTabType
{
    /// <summary>No tab is active (default file view).</summary>
    None,

    /// <summary>Tech debt items from .development/tech-debt/</summary>
    TechDebt,

    /// <summary>Architecture Decision Records from .development/reference/decisions/</summary>
    ADR,

    /// <summary>Specs from .development/specs/</summary>
    Specs,

    /// <summary>The config files tab.</summary>
    Config,

    /// <summary>The projects initialization tab.</summary>
    Projects,

    /// <summary>The global settings tab.</summary>
    Settings
}
