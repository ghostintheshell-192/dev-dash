using CommunityToolkit.Mvvm.ComponentModel;
using DevDash.Models;

namespace DevDash.ViewModels;

/// <summary>
/// ViewModel wrapper for a project in the initialization UI.
/// Adds selection state and configuration fields for scaffold initialization.
/// </summary>
public partial class ProjectInitItem : ViewModelBase
{
    public Project Project { get; }

    [ObservableProperty] private bool _isSelected;
    [ObservableProperty] private string _description = "";
    [ObservableProperty] private string _techStack = "";
    [ObservableProperty] private string _language;
    [ObservableProperty] private bool _isInitialized;
    [ObservableProperty] private string _statusMessage = "";

    public string Name => Project.Name;
    public string Path => Project.Path;
    public string? DetectedLanguage => Project.Language;

    public ProjectInitItem(Project project)
    {
        Project = project;
        _language = project.Language ?? "";
        _isInitialized = project.IsConfigured;

        // Don't auto-select already initialized projects
        _isSelected = !_isInitialized;
    }
}
