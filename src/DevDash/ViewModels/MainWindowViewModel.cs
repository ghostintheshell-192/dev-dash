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
    [ObservableProperty] private bool _showTerminal = true;
    [ObservableProperty] private bool _terminalExpanded;
    [ObservableProperty] private bool _showSettings;

    public ObservableCollection<ConfigFile> Configs { get; } = [];

    [ObservableProperty] private string? _workspacePath;
    [ObservableProperty] private string? _claudeConfigPath;
    [ObservableProperty] private string? _settingsFilePath;
    [ObservableProperty] private string _workspaceType = "coding";
    [ObservableProperty] private bool _isCodingWorkspace = true;
    [ObservableProperty] private bool _workspaceHasRules;
    [ObservableProperty] private bool _workspaceHasMemoryBank;
    [ObservableProperty] private bool _workspaceHasClaudeMd;

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

        SettingsFilePath = _appSettingsService.SettingsFilePath;
        var settings = _appSettingsService.Load();
        WorkspacePath = settings.WorkspacePath;
        ClaudeConfigPath = settings.ClaudeConfigPath;
        WorkspaceType = settings.WorkspaceType;
        IsCodingWorkspace = settings.WorkspaceType == "coding";
    }

    partial void OnIsCodingWorkspaceChanged(bool value) =>
        WorkspaceType = value ? "coding" : "writing";

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
    [RelayCommand] private void ToggleTerminal() => ShowTerminal = !ShowTerminal;
    [RelayCommand] private void ToggleTerminalExpanded() => TerminalExpanded = !TerminalExpanded;
    [RelayCommand] private void OpenSettings() => ShowSettings = true;
    [RelayCommand] private void CloseSettings() => ShowSettings = false;
    [RelayCommand] private void SetActiveTab(ActiveTabType tab) { if (ActiveTab != tab) ActiveTab = tab; }
    [RelayCommand] private void SelectWorkspace(Workspace workspace) => SelectedWorkspace = workspace;

    [RelayCommand]
    private async Task BrowseWorkspacePathAsync()
    {
        if (StorageProvider == null) return;
        var folders = await StorageProvider.OpenFolderPickerAsync(new FolderPickerOpenOptions
        {
            Title = "Select Workspace Folder",
            AllowMultiple = false
        });
        if (folders.Count > 0) WorkspacePath = folders[0].Path.LocalPath;
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
    private async Task SaveSettingsAsync()
    {
        var settings = new AppSettings
        {
            WorkspacePath = WorkspacePath,
            ClaudeConfigPath = ClaudeConfigPath,
            WorkspaceType = WorkspaceType
        };
        _appSettingsService.Save(settings);
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
        foreach (var ws in workspaces) Workspaces.Add(ws);

        if (Workspaces.Count > 0) SelectedWorkspace = Workspaces[0];

        await Task.CompletedTask;
    }
}
