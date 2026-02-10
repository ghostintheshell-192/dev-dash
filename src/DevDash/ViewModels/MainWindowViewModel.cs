using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
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
    [ObservableProperty] private ProjectInitializationViewModel? _projectInitializationVm;

    public ObservableCollection<FileTreeItemViewModel> FileTree { get; } = [];
    public ObservableCollection<FileTreeItemViewModel> DevelopmentTree { get; } = [];
    public ObservableCollection<FileTreeItemViewModel> DocsTree { get; } = [];

    // Specialized tab items
    public ObservableCollection<TechDebtItemViewModel> TechDebtItems { get; } = [];
    public ObservableCollection<AdrItemViewModel> AdrItems { get; } = [];
    public ObservableCollection<SpecItemViewModel> SpecItems { get; } = [];

    [ObservableProperty] private FileTreeItemViewModel? _selectedFile;
    [ObservableProperty] private string? _selectedFileContent;
    [ObservableProperty] private ActiveTabType _activeTab = ActiveTabType.TechDebt;

    // Document tabs
    public ObservableCollection<DocumentTabViewModel> OpenDocuments { get; } = [];
    [ObservableProperty] private DocumentTabViewModel? _selectedDocument;
    [ObservableProperty] private bool _showSidebar = true;
    [ObservableProperty] private bool _showSettings;

    public ObservableCollection<ConfigFile> Configs { get; } = [];

    // Multi-workspace support
    public ObservableCollection<WorkspaceConfigViewModel> ConfiguredWorkspaces { get; } = [];
    [ObservableProperty] private WorkspaceConfigViewModel? _selectedConfiguredWorkspace;
    [ObservableProperty] private string? _newWorkspacePath;
    [ObservableProperty] private string? _claudeConfigPath;
    [ObservableProperty] private string? _settingsFilePath;

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
            var vm = new WorkspaceConfigViewModel(ws);
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

        // Auto-save selection and reload projects
        SaveSettingsOnly();
        ReloadWorkspaces();
    }

    [RelayCommand]
    private void SelectConfiguredWorkspace(WorkspaceConfigViewModel workspace)
    {
        SelectedConfiguredWorkspace = workspace;
    }

    public Task InitializeAsync()
    {
        var workspaces = _workspaceService.GetWorkspaces();
        foreach (var ws in workspaces) Workspaces.Add(ws);

        if (Workspaces.Count > 0)
            SelectedWorkspace = Workspaces[0];

        var globalConfigs = _configurationService.GetGlobalConfigs();
        foreach (var config in globalConfigs) Configs.Add(config);

        return Task.CompletedTask;
    }

    partial void OnSelectedWorkspaceChanged(Workspace? value)
    {
        if (value != null)
        {
            _ = LoadProjectsAsync(value);

            // Initialize ProjectInitializationViewModel
            ProjectInitializationVm = new ProjectInitializationViewModel(_scaffoldService);
        }
    }

    private async Task LoadProjectsAsync(Workspace workspace)
    {
        Projects.Clear();
        FileTree.Clear();
        DevelopmentTree.Clear();
        DocsTree.Clear();
        TechDebtItems.Clear();
        AdrItems.Clear();
        SpecItems.Clear();
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
            LoadAllTrees(value.Project);
            LoadProjectConfigs(value.Project);
            LoadSpecializedTabs(value.Project);

            // Update project initialization panel
            ProjectInitializationVm?.SetProject(value.Project);
        }
    }

    private void LoadAllTrees(Project project)
    {
        LoadTreeInto(FileTree, _fileSystemService.GetPersonalTree(project.Path));
        LoadTreeInto(DevelopmentTree, _fileSystemService.GetDirectoryTree(project.Path, ".development"));
        LoadTreeInto(DocsTree, _fileSystemService.GetDirectoryTree(project.Path, "docs"));
    }

    private static void LoadTreeInto(ObservableCollection<FileTreeItemViewModel> target, PersonalFile? tree)
    {
        target.Clear();
        if (tree?.Children != null)
            foreach (var child in tree.Children) target.Add(new FileTreeItemViewModel(child));
    }

    private void LoadSpecializedTabs(Project project)
    {
        LoadTechDebtItems(project);
        LoadAdrItems(project);
        LoadSpecItems(project);
    }

    private void LoadTechDebtItems(Project project)
    {
        TechDebtItems.Clear();
        var dir = Path.Combine(project.Path, ".development", "tech-debt");
        if (!Directory.Exists(dir)) return;

        foreach (var file in Directory.GetFiles(dir, "*.md").OrderBy(Path.GetFileName))
        {
            var content = File.ReadAllText(file);
            var fm = FrontmatterData.Parse(content);
            TechDebtItems.Add(new TechDebtItemViewModel
            {
                FileName = Path.GetFileNameWithoutExtension(file),
                FullPath = file,
                Priority = fm?.Priority,
                Status = fm?.Status,
                Frontmatter = fm
            });
        }
    }

    private void LoadAdrItems(Project project)
    {
        AdrItems.Clear();
        var dir = Path.Combine(project.Path, ".development", "reference", "decisions");
        if (!Directory.Exists(dir)) return;

        var adrRegex = new Regex(@"^(\d+)-(.+)\.md$", RegexOptions.IgnoreCase);
        foreach (var file in Directory.GetFiles(dir, "*.md").OrderBy(Path.GetFileName))
        {
            var fileName = Path.GetFileName(file);
            var match = adrRegex.Match(fileName);
            if (!match.Success) continue;

            var content = File.ReadAllText(file);
            var fm = FrontmatterData.Parse(content);
            AdrItems.Add(new AdrItemViewModel
            {
                Number = match.Groups[1].Value,
                Title = match.Groups[2].Value.Replace('-', ' '),
                FullPath = file,
                Frontmatter = fm
            });
        }
    }

    private void LoadSpecItems(Project project)
    {
        SpecItems.Clear();
        var dir = Path.Combine(project.Path, ".development", "specs");
        if (!Directory.Exists(dir)) return;

        // Direct files go in "root" group
        foreach (var file in Directory.GetFiles(dir, "*.md").OrderBy(Path.GetFileName))
        {
            var content = File.ReadAllText(file);
            var fm = FrontmatterData.Parse(content);
            SpecItems.Add(new SpecItemViewModel
            {
                FileName = Path.GetFileNameWithoutExtension(file),
                FullPath = file,
                Group = "specs",
                Priority = fm?.Priority,
                Status = fm?.Status,
                Frontmatter = fm
            });
        }

        // Subdirectories as groups
        foreach (var subdir in Directory.GetDirectories(dir).OrderBy(Path.GetFileName))
        {
            var groupName = Path.GetFileName(subdir);
            foreach (var file in Directory.GetFiles(subdir, "*.md").OrderBy(Path.GetFileName))
            {
                var content = File.ReadAllText(file);
                var fm = FrontmatterData.Parse(content);
                SpecItems.Add(new SpecItemViewModel
                {
                    FileName = Path.GetFileNameWithoutExtension(file),
                    FullPath = file,
                    Group = groupName,
                    Priority = fm?.Priority,
                    Status = fm?.Status,
                    Frontmatter = fm
                });
            }
        }
    }

    private void LoadProjectConfigs(Project project)
    {
        // TODO: Display project-level configs in Config tab
        _ = _configurationService.GetProjectConfigs(project.Path);
    }

    partial void OnSelectedFileChanged(FileTreeItemViewModel? value)
    {
        if (value != null && value.IsFile)
            _ = OpenDocumentFromSidebarAsync(value.File);
    }

    private async Task OpenDocumentFromSidebarAsync(PersonalFile file)
    {
        var ownerTab = InferTabFromPath(file.FullPath);
        await OpenDocumentAsync(file, ownerTab);

        // Switch to the inferred tab so the document is visible
        if (ActiveTab != ownerTab)
        {
            ActiveTab = ownerTab;
            OnPropertyChanged(nameof(ShowTabList));
            OnPropertyChanged(nameof(CurrentTabDocuments));
            OnPropertyChanged(nameof(HasPersonalDocs));
            OnPropertyChanged(nameof(HasDevelopmentDocs));
            OnPropertyChanged(nameof(HasDocsDocs));
        }
    }

    private ActiveTabType InferTabFromPath(string filePath)
    {
        var normalized = filePath.Replace('\\', '/');
        if (normalized.Contains("/.personal/") || normalized.Contains("\\.personal\\"))
            return ActiveTabType.Personal;
        if (normalized.Contains("/.development/") || normalized.Contains("\\.development\\"))
            return ActiveTabType.Development;
        if (normalized.Contains("/docs/") || normalized.Contains("\\docs\\"))
            return ActiveTabType.Docs;
        // Fallback: use current active tab
        return ActiveTab;
    }

    private async Task OpenDocumentAsync(PersonalFile file, ActiveTabType ownerTab)
    {
        // Check if already open in the target tab
        var existing = OpenDocuments.FirstOrDefault(d => d.FilePath == file.FullPath && d.OwnerTab == ownerTab);
        if (existing != null)
        {
            SelectedDocument = existing;
            return;
        }

        // Create new tab owned by the specified tab
        var doc = new DocumentTabViewModel(file) { OwnerTab = ownerTab };
        try
        {
            doc.RawContent = await _fileSystemService.ReadFileAsync(file.FullPath);
        }
        catch
        {
            doc.RawContent = $"Error loading file: {file.FullPath}";
        }

        OpenDocuments.Add(doc);
        SelectedDocument = doc;
        NotifyDocumentCollectionChanged();
    }

    [RelayCommand]
    private async Task OpenDocumentByPathAsync(string? path)
    {
        if (string.IsNullOrEmpty(path) || !File.Exists(path)) return;
        var file = new PersonalFile
        {
            Name = Path.GetFileName(path),
            Path = path,
            FullPath = path,
            Type = FileType.File
        };
        await OpenDocumentAsync(file, ActiveTab);
    }

    /// <summary>Documents belonging to the currently active tab.</summary>
    public IEnumerable<DocumentTabViewModel> CurrentTabDocuments =>
        OpenDocuments.Where(d => d.OwnerTab == ActiveTab);

    /// <summary>True when the tab list (or panel) should be shown instead of the document viewer.</summary>
    public bool ShowTabList =>
        ActiveTab is ActiveTabType.Projects or ActiveTabType.Settings
        || SelectedDocument == null
        || SelectedDocument.OwnerTab != ActiveTab;

    // Dynamic tab visibility — shown only when they have open documents
    public bool HasPersonalDocs => OpenDocuments.Any(d => d.OwnerTab == ActiveTabType.Personal);
    public bool HasDevelopmentDocs => OpenDocuments.Any(d => d.OwnerTab == ActiveTabType.Development);
    public bool HasDocsDocs => OpenDocuments.Any(d => d.OwnerTab == ActiveTabType.Docs);

    private void NotifyDocumentCollectionChanged()
    {
        OnPropertyChanged(nameof(ShowTabList));
        OnPropertyChanged(nameof(CurrentTabDocuments));
        OnPropertyChanged(nameof(HasPersonalDocs));
        OnPropertyChanged(nameof(HasDevelopmentDocs));
        OnPropertyChanged(nameof(HasDocsDocs));
    }

    partial void OnSelectedDocumentChanged(DocumentTabViewModel? value)
    {
        foreach (var doc in OpenDocuments)
            doc.IsSelected = doc == value;

        // Keep SelectedFileContent in sync for backwards compatibility
        SelectedFileContent = value?.Content;
        NotifyDocumentCollectionChanged();
    }

    [RelayCommand]
    private void CloseDocument(DocumentTabViewModel doc)
    {
        var ownerTab = doc.OwnerTab;
        OpenDocuments.Remove(doc);

        // Select another document from the same tab, or null to show the list
        var nextInTab = OpenDocuments.LastOrDefault(d => d.OwnerTab == ownerTab);
        if (SelectedDocument == doc || doc.IsSelected)
        {
            SelectedDocument = nextInTab;
        }

        NotifyDocumentCollectionChanged();

        // If we closed the last doc of a dynamic tab, switch away
        if (ownerTab is ActiveTabType.Personal or ActiveTabType.Development or ActiveTabType.Docs
            && !OpenDocuments.Any(d => d.OwnerTab == ownerTab)
            && ActiveTab == ownerTab)
        {
            ActiveTab = ActiveTabType.TechDebt;
            SelectedDocument = null;
            NotifyDocumentCollectionChanged();
        }
    }

    [RelayCommand]
    private void CloseAllDocuments()
    {
        OpenDocuments.Clear();
        SelectedDocument = null;
        NotifyDocumentCollectionChanged();
    }

    [RelayCommand]
    private void CloseOtherDocuments(DocumentTabViewModel doc)
    {
        var toKeep = doc;
        OpenDocuments.Clear();
        OpenDocuments.Add(toKeep);
        SelectedDocument = toKeep;
        NotifyDocumentCollectionChanged();
    }

    [RelayCommand] private void ToggleSidebar() => ShowSidebar = !ShowSidebar;
    [RelayCommand] private void OpenSettings() => ShowSettings = true;
    [RelayCommand] private void CloseSettings() => ShowSettings = false;
    [RelayCommand]
    private void SetActiveTab(ActiveTabType tab)
    {
        if (ActiveTab == tab) return;
        ActiveTab = tab;

        // Restore the selected document for this tab (if any)
        var docForTab = OpenDocuments.FirstOrDefault(d => d.OwnerTab == tab && d.IsSelected)
                        ?? OpenDocuments.LastOrDefault(d => d.OwnerTab == tab);
        SelectedDocument = docForTab; // null => shows the list

        OnPropertyChanged(nameof(ShowTabList));
        OnPropertyChanged(nameof(CurrentTabDocuments));
    }
    [RelayCommand] private void SelectWorkspace(Workspace workspace) => SelectedWorkspace = workspace;

    [RelayCommand]
    private async Task HandleMarkdownLinkAsync(string? url)
    {
        if (string.IsNullOrEmpty(url)) return;

        // External URL - open in browser
        if (url.StartsWith("http://") || url.StartsWith("https://"))
        {
            OpenUrlInBrowser(url);
            return;
        }

        // Relative link - try to open as document
        if (SelectedDocument != null && SelectedProject != null)
        {
            // Resolve relative to current document's directory
            var currentDir = Path.GetDirectoryName(SelectedDocument.FilePath) ?? SelectedProject.Project.Path;
            var targetPath = Path.GetFullPath(Path.Combine(currentDir, url));

            if (File.Exists(targetPath))
            {
                var file = new PersonalFile
                {
                    Name = Path.GetFileName(targetPath),
                    Path = targetPath,
                    FullPath = targetPath,
                    Type = FileType.File
                };
                // Keep same owner tab as the document containing the link
                await OpenDocumentAsync(file, SelectedDocument.OwnerTab);
                return;
            }
        }

        // Fallback: try to open as URL anyway
        OpenUrlInBrowser(url);
    }

    private static void OpenUrlInBrowser(string url)
    {
        try
        {
            // Cross-platform URL opening
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                Process.Start(new ProcessStartInfo(url) { UseShellExecute = true });
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                Process.Start("open", url);
            }
            else
            {
                Process.Start("xdg-open", url);
            }
        }
        catch
        {
            // Silently fail if browser can't be opened
        }
    }

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

        var vm = new WorkspaceConfigViewModel(config);
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
    private void SaveSettings()
    {
        SaveSettingsOnly();
        ReloadWorkspaces();
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

    private void ReloadWorkspaces()
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
    }
}

/// <summary>
/// ViewModel for a configured workspace in settings.
/// </summary>
public partial class WorkspaceConfigViewModel : ViewModelBase
{
    public string Path { get; }
    [ObservableProperty] private string _name;
    [ObservableProperty] private bool _isCoding = true;
    [ObservableProperty] private bool _isSelected;

    public WorkspaceConfigViewModel(WorkspaceConfig config)
    {
        Path = config.Path;
        Name = config.Name;
        IsCoding = config.Type == "coding";
    }
}
