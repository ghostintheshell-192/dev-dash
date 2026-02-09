using System;
using System.Text.RegularExpressions;
using CommunityToolkit.Mvvm.ComponentModel;
using DevDash.Models;

namespace DevDash.ViewModels;

/// <summary>
/// Represents an open document tab.
/// </summary>
public partial class DocumentTabViewModel : ViewModelBase
{
    private static readonly Regex FrontmatterRegex = new(
        @"^---\s*\r?\n.*?\r?\n---\s*\r?\n",
        RegexOptions.Singleline | RegexOptions.Compiled);

    [ObservableProperty] private string _filePath;
    [ObservableProperty] private string _fileName;
    [ObservableProperty] private string _rawContent = string.Empty;
    [ObservableProperty] private bool _isSelected;
    [ObservableProperty] private bool _isMarkdown;

    /// <summary>
    /// Content for rendering (frontmatter stripped for markdown files).
    /// </summary>
    public string Content => IsMarkdown ? StripFrontmatter(_rawContent) : _rawContent;

    public DocumentTabViewModel(PersonalFile file)
    {
        FilePath = file.FullPath;
        FileName = file.Name;
        IsMarkdown = file.Name.EndsWith(".md", System.StringComparison.OrdinalIgnoreCase);
    }

    public DocumentTabViewModel(string filePath, string fileName)
    {
        FilePath = filePath;
        FileName = fileName;
        IsMarkdown = fileName.EndsWith(".md", System.StringComparison.OrdinalIgnoreCase);
    }

    partial void OnRawContentChanged(string value)
    {
        OnPropertyChanged(nameof(Content));
    }

    private static string StripFrontmatter(string content)
    {
        if (string.IsNullOrEmpty(content))
            return content;

        return FrontmatterRegex.Replace(content, string.Empty);
    }
}
