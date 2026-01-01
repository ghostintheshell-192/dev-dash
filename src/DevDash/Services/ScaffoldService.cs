using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace DevDash.Services;

/// <summary>
/// Service for creating workspace configuration scaffolds from embedded resources.
/// </summary>
public class ScaffoldService : IScaffoldService
{
    private const string ScaffoldPrefix = "workspace_scaffold";

    public async Task<ScaffoldResult> ApplyWorkspaceScaffoldAsync(string workspacePath)
    {
        if (string.IsNullOrWhiteSpace(workspacePath))
            return new ScaffoldResult(0, 0, 0);

        var assembly = Assembly.GetExecutingAssembly();
        var resourceNames = assembly.GetManifestResourceNames()
            .Where(n => n.Contains(ScaffoldPrefix))
            .ToList();

        var filesCreated = 0;
        var filesUpdated = 0;
        var filesSkipped = 0;

        foreach (var resourceName in resourceNames)
        {
            var relativePath = ExtractRelativePath(resourceName);
            if (string.IsNullOrEmpty(relativePath))
            {
                filesSkipped++;
                continue;
            }

            var targetPath = Path.Combine(workspacePath, relativePath);
            var content = await ReadEmbeddedResourceAsync(assembly, resourceName);

            if (content == null)
            {
                filesSkipped++;
                continue;
            }

            // Skip .gitkeep files - just ensure directory exists
            if (Path.GetFileName(relativePath) == ".gitkeep")
            {
                var dir = Path.GetDirectoryName(targetPath);
                if (!string.IsNullOrEmpty(dir))
                    Directory.CreateDirectory(dir);
                filesSkipped++;
                continue;
            }

            var fileExists = File.Exists(targetPath);

            // Ensure directory exists
            var directory = Path.GetDirectoryName(targetPath);
            if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
                Directory.CreateDirectory(directory);

            // Write file (always overwrite - these are system files, not user files)
            try
            {
                await File.WriteAllTextAsync(targetPath, content, Encoding.UTF8);
                if (fileExists)
                    filesUpdated++;
                else
                    filesCreated++;
            }
            catch
            {
                filesSkipped++;
            }
        }

        return new ScaffoldResult(filesCreated, filesUpdated, filesSkipped);
    }

    /// <summary>
    /// Extracts the relative file path from an embedded resource name.
    /// .NET embedded resource naming:
    /// - Slashes become dots
    /// - Leading dots in names get an extra dot prefix (e.g., .claude -> ..claude)
    /// - Hyphens in directories become underscores (session-handoff -> session_handoff)
    ///
    /// Example: "DevDash.rsrc.workspace_scaffold..claude.settings.json"
    ///       -> ".claude/settings.json"
    /// </summary>
    private static string? ExtractRelativePath(string resourceName)
    {
        // Find the scaffold prefix position
        var prefixIndex = resourceName.IndexOf(ScaffoldPrefix, StringComparison.Ordinal);
        if (prefixIndex < 0)
            return null;

        // Get everything after "workspace_scaffold."
        var afterPrefix = resourceName[(prefixIndex + ScaffoldPrefix.Length)..];
        if (afterPrefix.StartsWith("."))
            afterPrefix = afterPrefix[1..];

        if (string.IsNullOrEmpty(afterPrefix))
            return null;

        // Split by dots, but we need to handle the special cases:
        // - Double dots (..) indicate a leading dot in the original name
        // - The last segment is the file extension
        // - Second-to-last segment is the filename (without extension)

        // Rebuild path by processing the string
        var result = new List<string>();
        var current = afterPrefix;

        while (!string.IsNullOrEmpty(current))
        {
            // Check for double dot (hidden folder/file indicator)
            if (current.StartsWith("."))
            {
                // This is a hidden item, find the end of this segment
                var nextDot = current.IndexOf('.', 1);
                if (nextDot < 0)
                {
                    // Rest is the hidden item
                    result.Add(RestoreOriginalName(current));
                    break;
                }
                else
                {
                    result.Add(RestoreOriginalName(current[..nextDot]));
                    current = current[(nextDot + 1)..];
                }
            }
            else
            {
                // Normal segment
                var nextDot = current.IndexOf('.');
                if (nextDot < 0)
                {
                    result.Add(RestoreOriginalName(current));
                    break;
                }
                else
                {
                    result.Add(RestoreOriginalName(current[..nextDot]));
                    current = current[(nextDot + 1)..];
                }
            }
        }

        if (result.Count < 2)
            return null;

        // Handle special case: hidden files like .gitkeep (last element starts with .)
        // In this case, the last element IS the filename, not an extension
        string filename;
        List<string> dirParts;

        if (result[^1].StartsWith("."))
        {
            // Last element is a hidden file (e.g., .gitkeep)
            filename = result[^1];
            dirParts = result.Take(result.Count - 1).ToList();
        }
        else
        {
            // Normal case: last is extension, second-to-last is filename base
            var extension = result[^1];
            var filenameBase = result[^2];
            filename = filenameBase + "." + extension;
            dirParts = result.Take(result.Count - 2).ToList();
        }

        if (dirParts.Count == 0)
            return filename;

        var directoryPath = string.Join(Path.DirectorySeparatorChar.ToString(), dirParts);
        return Path.Combine(directoryPath, filename);
    }

    /// <summary>
    /// Restores original naming conventions from embedded resource naming.
    /// - Underscores become hyphens (e.g., memory_bank -> memory-bank)
    /// - Leading dots are preserved (e.g., .memory_bank -> .memory-bank)
    /// </summary>
    private static string RestoreOriginalName(string segment)
    {
        // Replace underscores with hyphens
        // This works for both hidden (.memory_bank -> .memory-bank)
        // and regular items (session_handoff -> session-handoff)
        return segment.Replace("_", "-");
    }

    private static async Task<string?> ReadEmbeddedResourceAsync(Assembly assembly, string resourceName)
    {
        try
        {
            using var stream = assembly.GetManifestResourceStream(resourceName);
            if (stream == null)
                return null;

            using var reader = new StreamReader(stream, Encoding.UTF8);
            return await reader.ReadToEndAsync();
        }
        catch
        {
            return null;
        }
    }
}
