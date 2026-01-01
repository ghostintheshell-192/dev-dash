using System.Collections.Generic;
using System.Threading.Tasks;

namespace DevDash.Services;

/// <summary>
/// Result of applying a scaffold to a workspace.
/// </summary>
public record ScaffoldResult(
    int FilesCreated,
    int FilesUpdated,
    int FilesSkipped
);

/// <summary>
/// Service for creating workspace configuration scaffolds.
/// </summary>
public interface IScaffoldService
{
    /// <summary>
    /// Applies the workspace scaffold to the specified path.
    /// Creates Claude Code configuration files (.claude/, .memory-bank/, scripts/).
    /// </summary>
    /// <param name="workspacePath">The workspace root path.</param>
    /// <returns>Result containing counts of files created, updated, and skipped.</returns>
    Task<ScaffoldResult> ApplyWorkspaceScaffoldAsync(string workspacePath);
}
