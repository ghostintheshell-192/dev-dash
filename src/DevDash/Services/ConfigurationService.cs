using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using DevDash.Models;

namespace DevDash.Services;

public class ConfigurationService : IConfigurationService
{
    private readonly IFileSystemService _fileSystem;
    private readonly string _homeDir;

    public ConfigurationService(IFileSystemService fileSystem)
    {
        _fileSystem = fileSystem;
        _homeDir = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
    }

    public IReadOnlyList<ConfigFile> GetGlobalConfigs()
    {
        var configs = new List<ConfigFile>();
        var claudeDir = Path.Combine(_homeDir, ".claude");

        // CLAUDE.md principale
        var claudeMd = Path.Combine(claudeDir, "CLAUDE.md");
        if (_fileSystem.FileExists(claudeMd))
        {
            configs.Add(new ConfigFile
            {
                Name = "Claude Global",
                Path = claudeMd,
                Level = ConfigLevel.Global,
                IconName = "Zap"
            });
        }

        // Bootstrap files
        var bootstrapCoding = Path.Combine(claudeDir, "bootstrap-coding.md");
        if (_fileSystem.FileExists(bootstrapCoding))
        {
            configs.Add(new ConfigFile
            {
                Name = "Bootstrap Coding",
                Path = bootstrapCoding,
                Level = ConfigLevel.Global,
                IconName = "Terminal"
            });
        }

        var bootstrapWriting = Path.Combine(claudeDir, "bootstrap-writing.md");
        if (_fileSystem.FileExists(bootstrapWriting))
        {
            configs.Add(new ConfigFile
            {
                Name = "Bootstrap Writing",
                Path = bootstrapWriting,
                Level = ConfigLevel.Global,
                IconName = "BookOpen"
            });
        }

        // Settings
        var settingsJson = Path.Combine(claudeDir, "settings.json");
        if (_fileSystem.FileExists(settingsJson))
        {
            configs.Add(new ConfigFile
            {
                Name = "Settings",
                Path = settingsJson,
                Level = ConfigLevel.Global,
                IconName = "Settings"
            });
        }

        return configs.AsReadOnly();
    }

    public IReadOnlyList<ConfigFile> GetWorkspaceConfigs(string workspacePath)
    {
        var configs = new List<ConfigFile>();

        // CLAUDE.md del workspace
        var claudeMd = Path.Combine(workspacePath, "CLAUDE.md");
        if (_fileSystem.FileExists(claudeMd))
        {
            configs.Add(new ConfigFile
            {
                Name = "Workspace Rules",
                Path = claudeMd,
                Level = ConfigLevel.Workspace,
                IconName = "Folder"
            });
        }

        // .claude/ del workspace
        var claudeDir = Path.Combine(workspacePath, ".claude");
        if (_fileSystem.DirectoryExists(claudeDir))
        {
            // Leggi regole dal folder rules/
            var rulesDir = Path.Combine(claudeDir, "rules");
            if (_fileSystem.DirectoryExists(rulesDir))
            {
                foreach (var file in _fileSystem.GetFiles(rulesDir, "*.md"))
                {
                    configs.Add(new ConfigFile
                    {
                        Name = Path.GetFileNameWithoutExtension(file),
                        Path = file,
                        Level = ConfigLevel.Workspace,
                        IconName = "FileText"
                    });
                }
            }
        }

        return configs.AsReadOnly();
    }

    public IReadOnlyList<ConfigFile> GetProjectConfigs(string projectPath)
    {
        var configs = new List<ConfigFile>();

        // CLAUDE.md del progetto
        var claudeMd = Path.Combine(projectPath, "CLAUDE.md");
        if (_fileSystem.FileExists(claudeMd))
        {
            configs.Add(new ConfigFile
            {
                Name = "Project Config",
                Path = claudeMd,
                Level = ConfigLevel.Project,
                IconName = "FileText"
            });
        }

        // .claude/ del progetto
        var claudeDir = Path.Combine(projectPath, ".claude");
        if (_fileSystem.DirectoryExists(claudeDir))
        {
            var rulesDir = Path.Combine(claudeDir, "rules");
            if (_fileSystem.DirectoryExists(rulesDir))
            {
                foreach (var file in _fileSystem.GetFiles(rulesDir, "*.md"))
                {
                    configs.Add(new ConfigFile
                    {
                        Name = Path.GetFileNameWithoutExtension(file),
                        Path = file,
                        Level = ConfigLevel.Project,
                        IconName = "FileText"
                    });
                }
            }
        }

        return configs.AsReadOnly();
    }

    public async Task<string> ReadConfigAsync(ConfigFile config)
    {
        return await _fileSystem.ReadFileAsync(config.Path);
    }
}
