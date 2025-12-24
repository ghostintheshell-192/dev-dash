using DevDash.Models;

namespace DevDash.Services;

/// <summary>
/// Service for reading and writing application settings.
/// </summary>
public interface IAppSettingsService
{
    /// <summary>
    /// Loads the application settings from disk.
    /// Returns default settings if the file doesn't exist.
    /// </summary>
    AppSettings Load();

    /// <summary>
    /// Saves the application settings to disk.
    /// </summary>
    void Save(AppSettings settings);

    /// <summary>
    /// Gets the path where the settings file is stored.
    /// </summary>
    string SettingsFilePath { get; }
}
