using System;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DevDash.Models;
using DevDash.Services;

namespace DevDash.ViewModels;

/// <summary>
/// ViewModel for the project initialization panel.
/// Initializes a single selected project with Claude Code configuration.
/// </summary>
public partial class ProjectInitializationViewModel : ViewModelBase
{
    private readonly IScaffoldService _scaffoldService;

    [ObservableProperty] private Project? _currentProject;
    [ObservableProperty] private string _projectDescription = "";
    [ObservableProperty] private string _projectTechStack = "";
    [ObservableProperty] private string _projectLanguage = "";
    [ObservableProperty] private bool _isProcessing;
    [ObservableProperty] private string _statusMessage = "";
    [ObservableProperty] private bool _isInitialized;

    public string? ProjectName => CurrentProject?.Name;
    public string? ProjectPath => CurrentProject?.Path;
    public string? DetectedLanguage => CurrentProject?.Language;
    public bool HasProject => CurrentProject != null;

    public ProjectInitializationViewModel(IScaffoldService scaffoldService)
    {
        _scaffoldService = scaffoldService;
    }

    public void SetProject(Project? project)
    {
        CurrentProject = project;

        if (project != null)
        {
            ProjectLanguage = project.Language ?? "";
            IsInitialized = project.IsConfigured;
            StatusMessage = "";
            ProjectDescription = "";
            ProjectTechStack = "";
        }

        OnPropertyChanged(nameof(ProjectName));
        OnPropertyChanged(nameof(ProjectPath));
        OnPropertyChanged(nameof(DetectedLanguage));
        OnPropertyChanged(nameof(HasProject));
    }

    [RelayCommand(CanExecute = nameof(CanInitialize))]
    private async Task InitializeProjectAsync()
    {
        if (CurrentProject == null) return;

        IsProcessing = true;
        StatusMessage = "";

        try
        {
            var config = new ProjectScaffoldConfig
            {
                ProjectName = CurrentProject.Name,
                ProjectDescription = string.IsNullOrWhiteSpace(ProjectDescription)
                    ? "Project description to be added."
                    : ProjectDescription,
                TechStack = string.IsNullOrWhiteSpace(ProjectTechStack)
                    ? "Tech stack to be documented."
                    : ProjectTechStack,
                Language = string.IsNullOrWhiteSpace(ProjectLanguage)
                    ? (CurrentProject.Language ?? "")
                    : ProjectLanguage
            };

            var result = await _scaffoldService.ApplyProjectScaffoldAsync(CurrentProject.Path, config);

            IsInitialized = true;
            StatusMessage = $"✓ Successfully initialized! {result.FilesCreated} files created, {result.FilesUpdated} updated.";
        }
        catch (Exception ex)
        {
            StatusMessage = $"✗ Error: {ex.Message}";
        }
        finally
        {
            IsProcessing = false;
        }
    }

    private bool CanInitialize() => !IsProcessing && CurrentProject != null && !IsInitialized;

    partial void OnIsProcessingChanged(bool value)
    {
        InitializeProjectCommand.NotifyCanExecuteChanged();
    }

    partial void OnIsInitializedChanged(bool value)
    {
        InitializeProjectCommand.NotifyCanExecuteChanged();
    }

    partial void OnCurrentProjectChanged(Project? value)
    {
        InitializeProjectCommand.NotifyCanExecuteChanged();
    }
}
