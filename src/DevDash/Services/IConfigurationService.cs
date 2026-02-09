using System.Collections.Generic;
using System.Threading.Tasks;
using DevDash.Models;

namespace DevDash.Services;

/// <summary>
/// Servizio per gestione configurazioni Claude Code
/// </summary>
public interface IConfigurationService
{
    /// <summary>
    /// Ottiene i file di configurazione globali (~/.claude/)
    /// </summary>
    IReadOnlyList<ConfigFile> GetGlobalConfigs();

    /// <summary>
    /// Ottiene i file di configurazione workspace
    /// </summary>
    IReadOnlyList<ConfigFile> GetWorkspaceConfigs(string workspacePath);

    /// <summary>
    /// Ottiene i file di configurazione di un progetto
    /// </summary>
    IReadOnlyList<ConfigFile> GetProjectConfigs(string projectPath);

    /// <summary>
    /// Legge il contenuto di un file di configurazione
    /// </summary>
    Task<string> ReadConfigAsync(ConfigFile config);
}
