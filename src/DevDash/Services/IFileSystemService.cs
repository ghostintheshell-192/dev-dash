using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using DevDash.Models;

namespace DevDash.Services;

public interface IFileSystemService
{
    bool DirectoryExists(string path);
    bool FileExists(string path);
    Task<string> ReadFileAsync(string path);
    Task WriteFileAsync(string path, string content);
    IEnumerable<string> GetDirectories(string path);
    IEnumerable<string> GetFiles(string path, string pattern = "*");
    
    /// <summary>
    /// Gets a directory tree for any subdirectory relative to a base path.
    /// </summary>
    PersonalFile? GetDirectoryTree(string basePath, string subdirectory);

    PersonalFile? GetPersonalTree(string projectPath);
    PersonalFile? GetMemoryBankTree(string workspacePath);
    PersonalFile? GetRulesTree(string workspacePath);

    DateTime? GetLastModified(string path);
}
