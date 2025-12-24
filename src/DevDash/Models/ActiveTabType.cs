namespace DevDash.Models;

/// <summary>
/// Represents the currently active tab in the main content area.
/// </summary>
public enum ActiveTabType
{
    /// <summary>No tab is active (default file view).</summary>
    None,

    /// <summary>The .personal files tab.</summary>
    Personal,

    /// <summary>The docs tab.</summary>
    Docs,

    /// <summary>The issues tab.</summary>
    Issues,

    /// <summary>The config files tab.</summary>
    Config,

    /// <summary>The global settings tab.</summary>
    Settings
}
