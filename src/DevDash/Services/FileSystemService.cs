using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using DevDash.Models;

namespace DevDash.Services;

public class FileSystemService : IFileSystemService
{
    public bool DirectoryExists(string path) => Directory.Exists(path);

    public bool FileExists(string path) => File.Exists(path);

    public async Task<string> ReadFileAsync(string path)
    {
        return await File.ReadAllTextAsync(path);
    }

    public async Task WriteFileAsync(string path, string content)
    {
        await File.WriteAllTextAsync(path, content);
    }

    public IEnumerable<string> GetDirectories(string path)
    {
        if (!Directory.Exists(path))
            return Enumerable.Empty<string>();

        return Directory.GetDirectories(path)
            .Where(d => !Path.GetFileName(d).StartsWith('.') || Path.GetFileName(d) == ".personal")
            .OrderBy(d => Path.GetFileName(d));
    }

    public IEnumerable<string> GetFiles(string path, string pattern = "*")
    {
        if (!Directory.Exists(path))
            return Enumerable.Empty<string>();

        return Directory.GetFiles(path, pattern)
            .Where(f => !Path.GetFileName(f).StartsWith('.'))
            .OrderBy(f => Path.GetFileName(f));
    }

    /// <summary>
    /// Ottiene l'albero di .personal/ per un progetto
    /// </summary>
    public PersonalFile? GetPersonalTree(string projectPath)
    {
        var personalPath = Path.Combine(projectPath, ".personal");
        if (!Directory.Exists(personalPath))
            return null;

        return BuildTree(personalPath, ".personal");
    }

    /// <summary>
    /// Ottiene l'albero di .memory-bank/ per un workspace
    /// </summary>
    public PersonalFile? GetMemoryBankTree(string workspacePath)
    {
        var memoryBankPath = Path.Combine(workspacePath, ".memory-bank");
        if (!Directory.Exists(memoryBankPath))
            return null;

        return BuildTree(memoryBankPath, ".memory-bank");
    }

    /// <summary>
    /// Ottiene l'albero di .rules/ per un workspace
    /// </summary>
    public PersonalFile? GetRulesTree(string workspacePath)
    {
        var rulesPath = Path.Combine(workspacePath, ".rules");
        if (!Directory.Exists(rulesPath))
            return null;

        return BuildTree(rulesPath, ".rules");
    }

    private PersonalFile BuildTree(string fullPath, string relativePath)
    {
        var name = Path.GetFileName(fullPath);
        var isDirectory = Directory.Exists(fullPath);

        if (isDirectory)
        {
            var children = new ObservableCollection<PersonalFile>();

            // Prima i file, poi le cartelle (ordinati alfabeticamente)
            var files = Directory.GetFiles(fullPath)
                .Where(f => !Path.GetFileName(f).StartsWith('.'))
                .OrderBy(f => Path.GetFileName(f));

            var dirs = Directory.GetDirectories(fullPath)
                .Where(d => !Path.GetFileName(d).StartsWith('.'))
                .OrderBy(d => Path.GetFileName(d));

            foreach (var file in files)
            {
                var fileName = Path.GetFileName(file);
                children.Add(new PersonalFile
                {
                    Name = fileName,
                    Path = Path.Combine(relativePath, fileName),
                    FullPath = file,
                    Type = FileType.File,
                    Priority = DetectPriority(file),
                    Modified = GetLastModified(file)
                });
            }

            foreach (var dir in dirs)
            {
                var dirName = Path.GetFileName(dir);
                children.Add(BuildTree(dir, Path.Combine(relativePath, dirName)));
            }

            return new PersonalFile
            {
                Name = name,
                Path = relativePath,
                FullPath = fullPath,
                Type = FileType.Folder,
                Children = children
            };
        }

        return new PersonalFile
        {
            Name = name,
            Path = relativePath,
            FullPath = fullPath,
            Type = FileType.File,
            Priority = DetectPriority(fullPath),
            Modified = GetLastModified(fullPath)
        };
    }

    private Priority DetectPriority(string filePath)
    {
        try
        {
            if (!File.Exists(filePath))
                return Priority.None;

            var content = File.ReadAllText(filePath);
            if (content.Contains("priority: high", StringComparison.OrdinalIgnoreCase))
                return Priority.High;
            if (content.Contains("priority: medium", StringComparison.OrdinalIgnoreCase))
                return Priority.Medium;
            if (content.Contains("priority: low", StringComparison.OrdinalIgnoreCase))
                return Priority.Low;
        }
        catch
        {
            // Ignora errori di lettura
        }

        return Priority.None;
    }

    public DateTime? GetLastModified(string path)
    {
        try
        {
            if (File.Exists(path))
                return File.GetLastWriteTime(path);
            if (Directory.Exists(path))
                return Directory.GetLastWriteTime(path);
        }
        catch
        {
            // Ignora errori
        }

        return null;
    }
}
