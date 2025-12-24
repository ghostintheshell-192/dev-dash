using System.Collections.ObjectModel;
using System.Linq;
using System.Threading.Tasks;
using Avalonia.Platform.Storage;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DevDash.Models;
using DevDash.Services;

namespace DevDash.ViewModels;

public partial class MainWindowViewModel : ViewModelBase
{
    private readonly IWorkspaceService _workspaceService;
    private readonly IFileSystemService _fileSystemService;
    private readonly IConfigurationService _configurationService;
    private readonly IAppSettingsService _appSettingsService;

    // Storage provider for folder picker dialogs
    public IStorageProvider? StorageProvider { get; set; }

    // Sidebars
    public ObservableCollection<SidebarItem> Sidebars { get; } = [];

    // Workspaces
    public ObservableCollection<Workspace> Workspaces { get; } = [];

    [ObservableProperty]
    private Workspace? _selectedWorkspace;

    // Projects
    public ObservableCollection<ProjectViewModel> Projects { get; } = [];

    [ObservableProperty]
    private ProjectViewModel? _selectedProject;

    // File tree (.personal)
    public ObservableCollection<FileTreeItemViewModel> FileTree { get; } = [];

    [ObservableProperty]
    private FileTreeItemViewModel? _selectedFile;

    [ObservableProperty]
    private string? _selectedFileContent;

    // Tabs
    [ObservableProperty]
    private ActiveTabType _activeTab = ActiveTabType.None;

    // Panel visibility
    [ObservableProperty]
    private bool _showSidebar = true;

    [ObservableProperty]
    private bool _showTerminal = true;

    [ObservableProperty]
    private bool _terminalExpanded;

    [ObservableProperty]
    private bool _showSettings;

    // Configs
    public ObservableCollection<ConfigFile> Configs { get; } = [];

    // App Settings
    [ObservableProperty]
    private string? _workspacePath;

    [ObservableProperty]
    private string? _claudeConfigPath;

    [ObservableProperty]
    private string? _settingsFilePath;

    public MainWindowViewModel(
        IWorkspaceService workspaceService,
        IFileSystemService fileSystemService,
        IConfigurationService configurationService,
        IAppSettingsService appSettingsService)
    {
        _workspaceService = workspaceService;
        _fileSystemService = fileSystemService;
        _configurationService = configurationService;
        _appSettingsService = appSettingsService;

        // Load settings
        SettingsFilePath = _appSettingsService.SettingsFilePath;
        var settings = _appSettingsService.Load();
        WorkspacePath = settings.WorkspacePath;
        ClaudeConfigPath = settings.ClaudeConfigPath;
    }

    public async Task InitializeAsync()
    {
        // Carica workspaces
        var workspaces = _workspaceService.GetWorkspaces();
        foreach (var ws in workspaces)
        {
            Workspaces.Add(ws);
        }

        // Seleziona il primo workspace
        if (Workspaces.Count > 0)
        {
            SelectedWorkspace = Workspaces[0];
        }

        // Carica configs globali
        var globalConfigs = _configurationService.GetGlobalConfigs();
        foreach (var config in globalConfigs)
        {
            Configs.Add(config);
        }

        await Task.CompletedTask;
    }

    partial void OnSelectedWorkspaceChanged(Workspace? value)
    {
        if (value != null)
        {
            _ = LoadProjectsAsync(value);
        }
    }

    private async Task LoadProjectsAsync(Workspace workspace)
    {
        Projects.Clear();
        FileTree.Clear();
        SelectedProject = null;
        SelectedFile = null;
        SelectedFileContent = null;

        var projects = await _workspaceService.GetProjectsAsync(workspace);
        foreach (var project in projects)
        {
            Projects.Add(new ProjectViewModel(project));
        }

        // Seleziona il primo progetto con .personal
        var firstWithPersonal = Projects.FirstOrDefault(p => p.HasPersonal);
        if (firstWithPersonal != null)
        {
            SelectedProject = firstWithPersonal;
        }
        else if (Projects.Count > 0)
        {
            SelectedProject = Projects[0];
        }
    }

    partial void OnSelectedProjectChanged(ProjectViewModel? value)
    {
        // Deseleziona il precedente
        foreach (var p in Projects)
        {
            p.IsSelected = p == value;
        }

        if (value != null)
        {
            LoadFileTree(value.Project);
            LoadProjectConfigs(value.Project);
        }
    }

    private void LoadFileTree(Project project)
    {
        FileTree.Clear();
        var tree = _fileSystemService.GetPersonalTree(project.Path);

        if (tree?.Children != null)
        {
            foreach (var child in tree.Children)
            {
                FileTree.Add(new FileTreeItemViewModel(child));
            }
        }
    }

    private void LoadProjectConfigs(Project project)
    {
        // Aggiungi config del progetto (dopo quelle globali)
        var projectConfigs = _configurationService.GetProjectConfigs(project.Path);
        // Per ora non aggiungiamo - gestiamo in futuro il merge
    }

    partial void OnSelectedFileChanged(FileTreeItemViewModel? value)
    {
        if (value != null && value.IsFile)
        {
            _ = LoadFileContentAsync(value.File);
        }
        else
        {
            SelectedFileContent = null;
        }
    }

    private async Task LoadFileContentAsync(PersonalFile file)
    {
        try
        {
            SelectedFileContent = await _fileSystemService.ReadFileAsync(file.FullPath);
        }
        catch
        {
            SelectedFileContent = $"Errore nel caricamento del file: {file.FullPath}";
        }
    }

    [RelayCommand]
    private void ToggleSidebar()
    {
        ShowSidebar = !ShowSidebar;
    }

    [RelayCommand]
    private void ToggleTerminal()
    {
        ShowTerminal = !ShowTerminal;
    }

    [RelayCommand]
    private void ToggleTerminalExpanded()
    {
        TerminalExpanded = !TerminalExpanded;
    }

    [RelayCommand]
    private void OpenSettings()
    {
        ShowSettings = true;
    }

    [RelayCommand]
    private void CloseSettings()
    {
        ShowSettings = false;
    }

    [RelayCommand]
    private void SetActiveTab(ActiveTabType tab)
    {
        // Skip if already on this tab (avoids unnecessary UI updates)
        if (ActiveTab == tab) return;

        ActiveTab = tab;
    }

    [RelayCommand]
    private void SelectWorkspace(Workspace workspace)
    {
        SelectedWorkspace = workspace;
    }

    [RelayCommand]
    private async Task BrowseWorkspacePathAsync()
    {
        if (StorageProvider == null) return;

        var folders = await StorageProvider.OpenFolderPickerAsync(new FolderPickerOpenOptions
        {
            Title = "Select Workspace Folder",
            AllowMultiple = false
        });

        if (folders.Count > 0)
        {
            WorkspacePath = folders[0].Path.LocalPath;
        }
    }

    [RelayCommand]
    private async Task BrowseClaudeConfigPathAsync()
    {
        if (StorageProvider == null) return;

        var folders = await StorageProvider.OpenFolderPickerAsync(new FolderPickerOpenOptions
        {
            Title = "Select Claude Config Folder",
            AllowMultiple = false
        });

        if (folders.Count > 0)
        {
            ClaudeConfigPath = folders[0].Path.LocalPath;
        }
    }

    [RelayCommand]
    private async Task SaveSettingsAsync()
    {
        var settings = new AppSettings
        {
            WorkspacePath = WorkspacePath,
            ClaudeConfigPath = ClaudeConfigPath
        };
        _appSettingsService.Save(settings);

        // Reload workspaces with new path
        await ReloadWorkspacesAsync();
    }

    private async Task ReloadWorkspacesAsync()
    {
        Workspaces.Clear();
        Projects.Clear();
        FileTree.Clear();
        SelectedWorkspace = null;
        SelectedProject = null;
        SelectedFile = null;
        SelectedFileContent = null;

        var workspaces = _workspaceService.GetWorkspaces();
        foreach (var ws in workspaces)
        {
            Workspaces.Add(ws);
        }

        if (Workspaces.Count > 0)
        {
            SelectedWorkspace = Workspaces[0];
        }

        await Task.CompletedTask;
    }
}
