using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace DevDash.Models;

/// <summary>
/// Parsed frontmatter data from a markdown document.
/// </summary>
public record FrontmatterData
{
    private static readonly Regex FrontmatterBlockRegex = new(
        @"^---\s*\r?\n(.*?)\r?\n---\s*\r?\n",
        RegexOptions.Singleline | RegexOptions.Compiled);

    private static readonly Regex KeyValueRegex = new(
        @"^(\w[\w-]*):\s*(.+)$",
        RegexOptions.Multiline | RegexOptions.Compiled);

    public string? Title { get; init; }
    public string? Status { get; init; }
    public string? Priority { get; init; }
    public string? Date { get; init; }
    public IReadOnlyList<string> Tags { get; init; } = [];
    public IReadOnlyDictionary<string, string> Raw { get; init; } = new Dictionary<string, string>();

    public bool HasData => Raw.Count > 0;

    public static FrontmatterData? Parse(string? content)
    {
        if (string.IsNullOrEmpty(content))
            return null;

        var match = FrontmatterBlockRegex.Match(content);
        if (!match.Success)
            return null;

        var block = match.Groups[1].Value;
        var raw = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

        foreach (Match kvMatch in KeyValueRegex.Matches(block))
        {
            raw[kvMatch.Groups[1].Value] = kvMatch.Groups[2].Value.Trim();
        }

        if (raw.Count == 0)
            return null;

        raw.TryGetValue("title", out var title);
        raw.TryGetValue("status", out var status);
        raw.TryGetValue("priority", out var priority);
        raw.TryGetValue("date", out var date);

        var tags = new List<string>();
        if (raw.TryGetValue("tags", out var tagsStr))
        {
            // Handle "tag1, tag2" or "[tag1, tag2]"
            tagsStr = tagsStr.Trim('[', ']');
            foreach (var tag in tagsStr.Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
                tags.Add(tag);
        }

        return new FrontmatterData
        {
            Title = title,
            Status = status,
            Priority = priority,
            Date = date,
            Tags = tags,
            Raw = raw
        };
    }
}
