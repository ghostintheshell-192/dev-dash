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
    /// Ottiene l'albero di .personal/ per un progetto
    /// </summary>
    PersonalFile? GetPersonalTree(string projectPath);
    
    /// <summary>
    /// Ottiene l'albero di .memory-bank/ per un workspace
    /// </summary>
    PersonalFile? GetMemoryBankTree(string workspacePath);
    
    /// <summary>
    /// Ottiene l'albero di .rules/ per un workspace
    /// </summary>
    PersonalFile? GetRulesTree(string workspacePath);
    
    DateTime? GetLastModified(string path);
}
