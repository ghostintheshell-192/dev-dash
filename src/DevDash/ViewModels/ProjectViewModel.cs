using CommunityToolkit.Mvvm.ComponentModel;
using DevDash.Models;

namespace DevDash.ViewModels;

/// <summary>
/// ViewModel per un progetto nella lista sidebar
/// </summary>
public partial class ProjectViewModel : ViewModelBase
{
    [ObservableProperty]
    private bool _isSelected;

    public Project Project { get; }

    public string Name => Project.Name;
    public string Path => Project.Path;
    public string? Branch => Project.Branch;
    public bool HasPersonal => Project.HasPersonal;
    public bool HasDocs => Project.HasDocs;
    public string? Language => Project.Language;

    public ProjectViewModel(Project project)
    {
        Project = project;
    }
}
