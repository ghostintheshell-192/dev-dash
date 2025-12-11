using System.Collections.Generic;
using System.Threading.Tasks;
using DevDash.Models;

namespace DevDash.Services;

/// <summary>
/// Servizio per gestione workspace e progetti
/// </summary>
public interface IWorkspaceService
{
    /// <summary>
    /// Ottiene tutti i workspace configurati
    /// </summary>
    IReadOnlyList<Workspace> GetWorkspaces();

    /// <summary>
    /// Ottiene i progetti di un workspace
    /// </summary>
    Task<IReadOnlyList<Project>> GetProjectsAsync(Workspace workspace);

    /// <summary>
    /// Rileva il linguaggio di un progetto
    /// </summary>
    string? DetectLanguage(string projectPath);

    /// <summary>
    /// Ottiene il branch git corrente
    /// </summary>
    string? GetCurrentBranch(string projectPath);
}
