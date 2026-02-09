using System;
using System.Collections.ObjectModel;

namespace DevDash.Models;

/// <summary>
/// Rappresenta un file o folder nella struttura .personal
/// </summary>
public record PersonalFile
{
    public required string Name { get; init; }
    public required string Path { get; init; }
    public required string FullPath { get; init; }
    public required FileType Type { get; init; }
    public Priority Priority { get; init; } = Priority.None;
    public DateTime? Modified { get; init; }

    /// <summary>
    /// Figli (se Type == Folder)
    /// </summary>
    public ObservableCollection<PersonalFile>? Children { get; init; }

    public bool IsFolder => Type == FileType.Folder;
    public bool IsFile => Type == FileType.File;
}
