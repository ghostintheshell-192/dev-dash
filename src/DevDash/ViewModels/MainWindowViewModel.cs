using System.Collections.ObjectModel;
using System.IO;
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
    private readonly IScaffoldService _scaffoldService;

    public IStorageProvider? StorageProvider { get; set; }
    public ObservableCollection<SidebarItem> Sidebars { get; } = [];
    public ObservableCollection<Workspace> Workspaces { get; } = [];

    [ObservableProperty] private Workspace? _selectedWorkspace;

    public ObservableCollection<ProjectViewModel> Projects { get; } = [];

    [ObservableProperty] private ProjectViewModel? _selectedProject;

    public ObservableCollection<FileTreeItemViewModel> FileTree { get; } = [];

    [ObservableProperty] private FileTreeItemViewModel? _selectedFile;
    [ObservableProperty] private string? _selectedFileContent;
    [ObservableProperty] private ActiveTabType _activeTab = ActiveTabType.None;
    [ObservableProperty] private bool _showSidebar = true;
    [ObservableProperty] private bool _showSettings;

    public ObservableCollection<ConfigFile> Configs { get; } = [];

    // Multi-workspace support
    public ObservableCollection<WorkspaceConfigViewModel> ConfiguredWorkspaces { get; } = [];
    [ObservableProperty] private WorkspaceConfigViewModel? _selectedConfiguredWorkspace;
    [ObservableProperty] private string? _newWorkspacePath;
    [ObservableProperty] private string? _claudeConfigPath;
    [ObservableProperty] private string? _settingsFilePath;
    [ObservableProperty] private bool _workspaceHasRules;
    [ObservableProperty] private bool _workspaceHasMemoryBank;
    [ObservableProperty] private bool _workspaceHasClaudeMd;

    public MainWindowViewModel(
        IWorkspaceService workspaceService,
        IFileSystemService fileSystemService,
        IConfigurationService configurationService,
        IAppSettingsService appSettingsService,
        IScaffoldService scaffoldService)
    {
        _workspaceService = workspaceService;
        _fileSystemService = fileSystemService;
        _configurationService = configurationService;
        _appSettingsService = appSettingsService;
        _scaffoldService = scaffoldService;

        SettingsFilePath = _appSettingsService.SettingsFilePath;
        LoadSettings();
    }

    private void LoadSettings()
    {
        var settings = _appSettingsService.Load();
        ClaudeConfigPath = settings.ClaudeConfigPath;

        ConfiguredWorkspaces.Clear();
        foreach (var ws in settings.Workspaces)
        {
            var vm = new WorkspaceConfigViewModel(ws, _scaffoldService);
            ConfiguredWorkspaces.Add(vm);
        }

        if (settings.SelectedWorkspaceIndex >= 0 && settings.SelectedWorkspaceIndex < ConfiguredWorkspaces.Count)
        {
            SelectedConfiguredWorkspace = ConfiguredWorkspaces[settings.SelectedWorkspaceIndex];
        }
        else if (ConfiguredWorkspaces.Count > 0)
        {
            SelectedConfiguredWorkspace = ConfiguredWorkspaces[0];
        }
    }

    partial void OnSelectedConfiguredWorkspaceChanged(WorkspaceConfigViewModel? value)
    {
        // Update selection state on all workspaces
        foreach (var ws in ConfiguredWorkspaces)
            ws.IsSelected = ws == value;

        if (value != null)
        {
            value.RefreshStatus();
        }

        // Auto-save selection and reload projects
        SaveSettingsOnly();
        _ = ReloadWorkspacesAsync();
    }

    [RelayCommand]
    private void SelectConfiguredWorkspace(WorkspaceConfigViewModel workspace)
    {
        SelectedConfiguredWorkspace = workspace;
    }

    public async Task InitializeAsync()
    {
        var workspaces = _workspaceService.GetWorkspaces();
        foreach (var ws in workspaces) Workspaces.Add(ws);

        if (Workspaces.Count > 0)
        {
            SelectedWorkspace = Workspaces[0];
            UpdateWorkspaceStatus(Workspaces[0]);
        }

        var globalConfigs = _configurationService.GetGlobalConfigs();
        foreach (var config in globalConfigs) Configs.Add(config);

        await Task.CompletedTask;
    }

    private void UpdateWorkspaceStatus(Workspace ws)
    {
        WorkspaceHasRules = ws.HasRules;
        WorkspaceHasMemoryBank = ws.HasMemoryBank;
        WorkspaceHasClaudeMd = ws.HasClaudeMd;
    }

    partial void OnSelectedWorkspaceChanged(Workspace? value)
    {
        if (value != null)
        {
            UpdateWorkspaceStatus(value);
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
        foreach (var project in projects) Projects.Add(new ProjectViewModel(project));

        var firstWithPersonal = Projects.FirstOrDefault(p => p.HasPersonal);
        SelectedProject = firstWithPersonal ?? Projects.FirstOrDefault();
    }

    partial void OnSelectedProjectChanged(ProjectViewModel? value)
    {
        foreach (var p in Projects) p.IsSelected = p == value;
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
            foreach (var child in tree.Children) FileTree.Add(new FileTreeItemViewModel(child));
    }

    private void LoadProjectConfigs(Project project)
    {
        var projectConfigs = _configurationService.GetProjectConfigs(project.Path);
    }

    partial void OnSelectedFileChanged(FileTreeItemViewModel? value)
    {
        if (value != null && value.IsFile) _ = LoadFileContentAsync(value.File);
        else SelectedFileContent = null;
    }

    private async Task LoadFileContentAsync(PersonalFile file)
    {
        try { SelectedFileContent = await _fileSystemService.ReadFileAsync(file.FullPath); }
        catch { SelectedFileContent = "Errore nel caricamento del file: " + file.FullPath; }
    }

    [RelayCommand] private void ToggleSidebar() => ShowSidebar = !ShowSidebar;
    [RelayCommand] private void OpenSettings() => ShowSettings = true;
    [RelayCommand] private void CloseSettings() => ShowSettings = false;
    [RelayCommand] private void SetActiveTab(ActiveTabType tab) { if (ActiveTab != tab) ActiveTab = tab; }
    [RelayCommand] private void SelectWorkspace(Workspace workspace) => SelectedWorkspace = workspace;

    [RelayCommand]
    private async Task BrowseNewWorkspacePathAsync()
    {
        if (StorageProvider == null) return;
        var folders = await StorageProvider.OpenFolderPickerAsync(new FolderPickerOpenOptions
        {
            Title = "Select Workspace Folder",
            AllowMultiple = false
        });
        if (folders.Count > 0) NewWorkspacePath = folders[0].Path.LocalPath;
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
        if (folders.Count > 0) ClaudeConfigPath = folders[0].Path.LocalPath;
    }

    [RelayCommand]
    private void AddWorkspace()
    {
        if (string.IsNullOrWhiteSpace(NewWorkspacePath)) return;
        if (!Directory.Exists(NewWorkspacePath)) return;

        // Check if already exists
        if (ConfiguredWorkspaces.Any(w => w.Path == NewWorkspacePath)) return;

        var name = Path.GetFileName(NewWorkspacePath) ?? "Workspace";
        var config = new WorkspaceConfig
        {
            Path = NewWorkspacePath,
            Name = name,
            Type = "coding"
        };

        var vm = new WorkspaceConfigViewModel(config, _scaffoldService);
        ConfiguredWorkspaces.Add(vm);
        SelectedConfiguredWorkspace = vm;
        NewWorkspacePath = null;

        SaveSettingsOnly();
    }

    [RelayCommand]
    private void RemoveWorkspace(WorkspaceConfigViewModel? workspace)
    {
        if (workspace == null) return;
        ConfiguredWorkspaces.Remove(workspace);

        if (SelectedConfiguredWorkspace == workspace)
        {
            SelectedConfiguredWorkspace = ConfiguredWorkspaces.FirstOrDefault();
        }

        SaveSettingsOnly();
    }

    [RelayCommand]
    private async Task ApplyScaffoldAsync(WorkspaceConfigViewModel? workspace)
    {
        if (workspace == null || string.IsNullOrWhiteSpace(workspace.Path)) return;

        await _scaffoldService.ApplyWorkspaceScaffoldAsync(workspace.Path);
        workspace.RefreshStatus();
        await ReloadWorkspacesAsync();
    }

    [RelayCommand]
    private async Task SaveSettingsAsync()
    {
        SaveSettingsOnly();
        await ReloadWorkspacesAsync();
    }

    private void SaveSettingsOnly()
    {
        var workspaces = ConfiguredWorkspaces.Select(w => new WorkspaceConfig
        {
            Path = w.Path,
            Name = w.Name,
            Type = w.IsCoding ? "coding" : "writing"
        }).ToList();

        var selectedIndex = SelectedConfiguredWorkspace != null
            ? ConfiguredWorkspaces.IndexOf(SelectedConfiguredWorkspace)
            : 0;

        var settings = new AppSettings
        {
            Workspaces = workspaces,
            SelectedWorkspaceIndex = selectedIndex,
            ClaudeConfigPath = ClaudeConfigPath
        };

        _appSettingsService.Save(settings);
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
        foreach (var ws in workspaces) Workspaces.Add(ws);

        if (Workspaces.Count > 0) SelectedWorkspace = Workspaces[0];

        await Task.CompletedTask;
    }
}

/// <summary>
/// ViewModel for a configured workspace in settings.
/// </summary>
public partial class WorkspaceConfigViewModel : ViewModelBase
{
    private readonly IScaffoldService _scaffoldService;

    public string Path { get; }
    [ObservableProperty] private string _name;
    [ObservableProperty] private bool _isCoding = true;
    [ObservableProperty] private bool _isConfigured;
    [ObservableProperty] private bool _isSelected;
    [ObservableProperty] private string _statusText = "Not configured";

    public WorkspaceConfigViewModel(WorkspaceConfig config, IScaffoldService scaffoldService)
    {
        _scaffoldService = scaffoldService;
        Path = config.Path;
        Name = config.Name;
        IsCoding = config.Type == "coding";
        RefreshStatus();
    }

    public void RefreshStatus()
    {
        IsConfigured = _scaffoldService.IsWorkspaceConfigured(Path);
        StatusText = IsConfigured ? "Configured" : "Not configured";
    }

    partial void OnIsCodingChanged(bool value)
    {
        // Type changed, could trigger save in parent
    }
}
