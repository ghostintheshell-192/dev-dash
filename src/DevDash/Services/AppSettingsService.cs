using System;
using System.IO;
using System.Text.Json;
using DevDash.Models;

namespace DevDash.Services;

/// <summary>
/// Service for reading and writing application settings to a JSON file.
/// Settings are stored in the platform-appropriate application data directory.
/// </summary>
public class AppSettingsService : IAppSettingsService
{
    private const string AppName = "DevDash";
    private const string SettingsFileName = "settings.json";

    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase
    };

    public string SettingsFilePath { get; }

    public AppSettingsService()
    {
        var appDataPath = GetAppDataPath();
        SettingsFilePath = Path.Combine(appDataPath, SettingsFileName);
    }

    public AppSettings Load()
    {
        if (!File.Exists(SettingsFilePath))
        {
            return new AppSettings();
        }

        try
        {
            var json = File.ReadAllText(SettingsFilePath);
            return JsonSerializer.Deserialize<AppSettings>(json, JsonOptions) ?? new AppSettings();
        }
        catch (Exception)
        {
            // If the file is corrupted, return default settings
            return new AppSettings();
        }
    }

    public void Save(AppSettings settings)
    {
        var directory = Path.GetDirectoryName(SettingsFilePath);
        if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
        {
            Directory.CreateDirectory(directory);
        }

        var json = JsonSerializer.Serialize(settings, JsonOptions);
        File.WriteAllText(SettingsFilePath, json);
    }

    private static string GetAppDataPath()
    {
        // Cross-platform: uses appropriate location per OS
        // Windows: %APPDATA%\DevDash
        // Linux: ~/.config/DevDash
        // macOS: ~/Library/Application Support/DevDash
        var basePath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
        return Path.Combine(basePath, AppName);
    }
}
