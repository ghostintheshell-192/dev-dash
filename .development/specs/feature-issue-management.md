---
type: feature
priority: high
status: planned
category: productivity
related: [feature-markdown-rendering]
---

# Issue Management UI

## Overview

Sistema di gestione issue integrato che sfrutta i file markdown con frontmatter YAML già esistenti in `.personal/active/tech-debt/`, permettendo visualizzazione strutturata e editing rapido tramite GUI.

## User Story

Come utente, voglio:
- Vedere tutte le issue in una vista organizzata (kanban board o lista)
- Filtrare per status, priority, type
- Modificare rapidamente status/priority senza editare file manualmente
- Creare nuove issue da template
- Navigare facilmente tra issue e dettagli
- Collegare issue a commit/branch

## Existing Format

Hai già questo formato in SheetAtlas:

```markdown
---
type: performance
priority: medium
status: open
discovered: 2025-11-08
related: []
related_decision: null
report: null
---

# Issue Title

## Problem
...

## Analysis
...

## Possible Solutions
...
```

## UI Concepts

### View 1: Kanban Board (Primary)

```
┌─────────────┬─────────────┬─────────────┬─────────────┐
│ OPEN        │ IN PROGRESS │ BLOCKED     │ COMPLETED   │
├─────────────┼─────────────┼─────────────┼─────────────┤
│ ┌─────────┐ │ ┌─────────┐ │             │ ┌─────────┐ │
│ │ Issue 1 │ │ │ Issue 3 │ │             │ │ Issue 5 │ │
│ │ [perf]  │ │ │ [bug]   │ │             │ │ [feat]  │ │
│ │ HIGH    │ │ │ MEDIUM  │ │             │ │ LOW     │ │
│ └─────────┘ │ └─────────┘ │             │ └─────────┘ │
│             │             │             │             │
│ ┌─────────┐ │             │             │             │
│ │ Issue 2 │ │             │             │             │
│ │ [bug]   │ │             │             │             │
│ │ MEDIUM  │ │             │             │             │
│ └─────────┘ │             │             │             │
└─────────────┴─────────────┴─────────────┴─────────────┘

[+ New Issue]    [Filter: All ▼]    [Sort: Priority ▼]
```

**Interaction:**
- Drag & drop card → change status (aggiorna frontmatter)
- Click card → mostra dettagli con markdown rendering
- Double-click → edit mode
- Right-click → context menu (edit, delete, duplicate, link to commit)

### View 2: List View (Secondary)

```
┌──────────────────────────────────────────────────────┐
│ [Type ▼] [Priority ▼] [Status ▼]       [🔍 Search] │
├──────────────────────────────────────────────────────┤
│ ✓ │ cellmetadata-memory-waste       │ perf │ high │
│   │ row-comparison-sync-bug         │ bug  │ med  │
│   │ theme-change-bug                │ bug  │ med  │
│   │ sacellvalue-naming              │ refactor │ low │
├──────────────────────────────────────────────────────┤
│ Details: cellmetadata-memory-waste                   │
│ Status: [open ▼]  Priority: [high ▼]  Type: [perf ▼]│
│                                                       │
│ # CellMetadata Memory Waste                          │
│ Currently allocating entire...                       │
│ [rendered markdown]                                  │
└──────────────────────────────────────────────────────┘
```

### View 3: Detail Panel (Opens from Kanban/List)

```
┌──────────────────────────────────────────────────────┐
│ ← Back to Board             [Edit] [Delete] [...]   │
├──────────────────────────────────────────────────────┤
│ cellmetadata-memory-waste.md                         │
│                                                       │
│ Type: [performance ▼]  Priority: [high ▼]           │
│ Status: [open ▼]      Discovered: 2025-11-08        │
│ Related: (none)       Report: (none)                 │
│ [Save Metadata]                                      │
├──────────────────────────────────────────────────────┤
│ # CellMetadata Memory Waste for NumberFormat        │
│                                                       │
│ ## Problem                                           │
│ Currently allocating entire `CellMetadata`...        │
│ [rendered markdown body]                             │
│                                                       │
│ [Edit Document]                                      │
└──────────────────────────────────────────────────────┘
```

## Technical Implementation

### Data Model

```csharp
public class Issue : ObservableObject
{
    // From frontmatter
    [ObservableProperty]
    private string _type;  // bug, feature, refactor, performance, etc.

    [ObservableProperty]
    private string _priority;  // high, medium, low

    [ObservableProperty]
    private string _status;  // open, in_progress, blocked, completed

    [ObservableProperty]
    private DateTime _discovered;

    [ObservableProperty]
    private List<string> _related;

    [ObservableProperty]
    private string? _relatedDecision;

    [ObservableProperty]
    private string? _report;

    // From file
    public string FilePath { get; set; }
    public string FileName { get; set; }

    // From markdown body
    public string Title { get; set; }  // Estratto dal primo # heading
    public string MarkdownBody { get; set; }

    // Computed
    public string DisplayTitle => Title ?? FileName.Replace(".md", "");

    // Methods
    public void UpdateStatus(string newStatus)
    {
        Status = newStatus;
        SaveFrontmatter();  // Aggiorna solo YAML, non tocca body
    }

    public void UpdatePriority(string newPriority)
    {
        Priority = newPriority;
        SaveFrontmatter();
    }

    private void SaveFrontmatter()
    {
        var content = File.ReadAllText(FilePath);
        var (_, body) = ParseMarkdown(content);

        var yaml = new Serializer().Serialize(new
        {
            type = Type,
            priority = Priority,
            status = Status,
            discovered = Discovered,
            related = Related,
            related_decision = RelatedDecision,
            report = Report
        });

        var newContent = $"---\n{yaml}---\n{body}";
        File.WriteAllText(FilePath, newContent);
    }
}
```

### Service Layer

```csharp
public interface IIssueService
{
    Task<List<Issue>> LoadIssuesAsync(string projectPath);
    Task<Issue> CreateIssueAsync(string projectPath, IssueTemplate template);
    Task UpdateIssueAsync(Issue issue);
    Task DeleteIssueAsync(Issue issue);
    Task<List<Issue>> SearchIssuesAsync(string query, IssueFilter filter);
}

public class IssueService : IIssueService
{
    public async Task<List<Issue>> LoadIssuesAsync(string projectPath)
    {
        var issuesPath = Path.Combine(projectPath, ".personal/active/tech-debt");

        if (!Directory.Exists(issuesPath))
            return new List<Issue>();

        var files = Directory.GetFiles(issuesPath, "*.md", SearchOption.AllDirectories)
            .Where(f => !f.EndsWith("_TEMPLATE.md") && !f.EndsWith("README.md"));

        var issues = new List<Issue>();

        foreach (var file in files)
        {
            var content = await File.ReadAllTextAsync(file);
            var issue = ParseIssue(content, file);
            if (issue != null)
                issues.Add(issue);
        }

        return issues;
    }

    private Issue? ParseIssue(string content, string filePath)
    {
        var match = Regex.Match(content, @"^---\s*\n(.*?)\n---\s*\n(.*)$",
                                RegexOptions.Singleline);

        if (!match.Success)
            return null;

        try
        {
            var deserializer = new DeserializerBuilder().Build();
            var frontmatter = deserializer.Deserialize<Dictionary<string, object>>(
                match.Groups[1].Value);

            var body = match.Groups[2].Value;
            var titleMatch = Regex.Match(body, @"^#\s+(.+)$", RegexOptions.Multiline);

            return new Issue
            {
                FilePath = filePath,
                FileName = Path.GetFileName(filePath),
                Type = frontmatter.GetValueOrDefault("type")?.ToString() ?? "",
                Priority = frontmatter.GetValueOrDefault("priority")?.ToString() ?? "",
                Status = frontmatter.GetValueOrDefault("status")?.ToString() ?? "",
                Discovered = DateTime.Parse(
                    frontmatter.GetValueOrDefault("discovered")?.ToString() ?? DateTime.Now.ToString()),
                Related = (frontmatter.GetValueOrDefault("related") as List<object>)?
                    .Select(x => x.ToString()).ToList() ?? new List<string>(),
                RelatedDecision = frontmatter.GetValueOrDefault("related_decision")?.ToString(),
                Report = frontmatter.GetValueOrDefault("report")?.ToString(),
                Title = titleMatch.Success ? titleMatch.Groups[1].Value : null,
                MarkdownBody = body
            };
        }
        catch (Exception ex)
        {
            // Log error, skip malformed file
            return null;
        }
    }
}
```

### ViewModel

```csharp
public class IssuesViewModel : ViewModelBase
{
    private readonly IIssueService _issueService;

    [ObservableProperty]
    private ObservableCollection<Issue> _allIssues;

    [ObservableProperty]
    private ObservableCollection<Issue> _openIssues;

    [ObservableProperty]
    private ObservableCollection<Issue> _inProgressIssues;

    [ObservableProperty]
    private ObservableCollection<Issue> _blockedIssues;

    [ObservableProperty]
    private ObservableCollection<Issue> _completedIssues;

    [ObservableProperty]
    private Issue? _selectedIssue;

    [ObservableProperty]
    private string _searchQuery = "";

    [ObservableProperty]
    private IssueViewMode _viewMode = IssueViewMode.Kanban;

    public IssuesViewModel(IIssueService issueService)
    {
        _issueService = issueService;
    }

    public async Task LoadIssuesAsync(string projectPath)
    {
        AllIssues = new ObservableCollection<Issue>(
            await _issueService.LoadIssuesAsync(projectPath));

        GroupIssuesByStatus();
    }

    private void GroupIssuesByStatus()
    {
        OpenIssues = new ObservableCollection<Issue>(
            AllIssues.Where(i => i.Status == "open"));
        InProgressIssues = new ObservableCollection<Issue>(
            AllIssues.Where(i => i.Status == "in_progress"));
        BlockedIssues = new ObservableCollection<Issue>(
            AllIssues.Where(i => i.Status == "blocked"));
        CompletedIssues = new ObservableCollection<Issue>(
            AllIssues.Where(i => i.Status == "completed"));
    }

    [RelayCommand]
    private async Task MoveIssueAsync(Issue issue, string newStatus)
    {
        issue.UpdateStatus(newStatus);
        GroupIssuesByStatus();  // Re-group
    }

    [RelayCommand]
    private async Task CreateNewIssueAsync()
    {
        var template = IssueTemplate.Default;
        var issue = await _issueService.CreateIssueAsync(
            CurrentProject.Path, template);

        AllIssues.Add(issue);
        GroupIssuesByStatus();
        SelectedIssue = issue;
    }

    [RelayCommand]
    private async Task DeleteIssueAsync(Issue issue)
    {
        await _issueService.DeleteIssueAsync(issue);
        AllIssues.Remove(issue);
        GroupIssuesByStatus();
    }
}
```

## UI Implementation (Kanban Board)

```xml
<Grid RowDefinitions="Auto,*">
    <!-- Toolbar -->
    <Border Grid.Row="0" Background="#0f172a" Padding="16">
        <StackPanel Orientation="Horizontal" Spacing="16">
            <Button Command="{Binding CreateNewIssueCommand}">
                <StackPanel Orientation="Horizontal" Spacing="8">
                    <PathIcon Data="M12 5v14M5 12h14" Width="16" Height="16"/>
                    <TextBlock Text="New Issue"/>
                </StackPanel>
            </Button>

            <ComboBox SelectedItem="{Binding TypeFilter}"
                      ItemsSource="{Binding AvailableTypes}"
                      PlaceholderText="All Types"/>

            <ComboBox SelectedItem="{Binding PriorityFilter}"
                      ItemsSource="{Binding AvailablePriorities}"
                      PlaceholderText="All Priorities"/>

            <TextBox Text="{Binding SearchQuery}"
                     Watermark="Search issues..."/>
        </StackPanel>
    </Border>

    <!-- Kanban Board -->
    <Grid Grid.Row="1" ColumnDefinitions="*,*,*,*">
        <!-- OPEN Column -->
        <Border Grid.Column="0" Background="#1e293b" Margin="4">
            <DockPanel>
                <TextBlock DockPanel.Dock="Top"
                           Text="OPEN"
                           FontWeight="Bold"
                           Padding="16"/>

                <ScrollViewer>
                    <ItemsControl ItemsSource="{Binding OpenIssues}">
                        <ItemsControl.ItemTemplate>
                            <DataTemplate>
                                <views:IssueCard Issue="{Binding}"
                                                Command="{Binding $parent[UserControl].DataContext.SelectIssueCommand}"
                                                CommandParameter="{Binding}"/>
                            </DataTemplate>
                        </ItemsControl.ItemTemplate>
                    </ItemsControl>
                </ScrollViewer>
            </DockPanel>
        </Border>

        <!-- IN PROGRESS Column -->
        <Border Grid.Column="1" Background="#1e293b" Margin="4">
            <DockPanel>
                <TextBlock DockPanel.Dock="Top"
                           Text="IN PROGRESS"
                           FontWeight="Bold"
                           Padding="16"/>

                <ScrollViewer>
                    <ItemsControl ItemsSource="{Binding InProgressIssues}">
                        <ItemsControl.ItemTemplate>
                            <DataTemplate>
                                <views:IssueCard Issue="{Binding}"/>
                            </DataTemplate>
                        </ItemsControl.ItemTemplate>
                    </ItemsControl>
                </ScrollViewer>
            </DockPanel>
        </Border>

        <!-- BLOCKED, COMPLETED columns... -->
    </Grid>
</Grid>
```

## Issue Card Component

```xml
<!-- IssueCard.axaml -->
<Border Background="#334155"
        CornerRadius="8"
        Padding="12"
        Margin="8"
        Classes.high="{Binding Priority, Converter={StaticResource PriorityHighConverter}}"
        Classes.medium="{Binding Priority, Converter={StaticResource PriorityMediumConverter}}">

    <StackPanel Spacing="8">
        <!-- Title -->
        <TextBlock Text="{Binding DisplayTitle}"
                   FontWeight="SemiBold"
                   TextWrapping="Wrap"/>

        <!-- Badges -->
        <StackPanel Orientation="Horizontal" Spacing="4">
            <Border Background="#0ea5e9" CornerRadius="4" Padding="4,2">
                <TextBlock Text="{Binding Type}" FontSize="10"/>
            </Border>

            <Border Background="#f59e0b" CornerRadius="4" Padding="4,2"
                    IsVisible="{Binding Priority, Converter={StaticResource IsHighPriority}}">
                <TextBlock Text="{Binding Priority}" FontSize="10"/>
            </Border>
        </StackPanel>

        <!-- Date -->
        <TextBlock Text="{Binding Discovered, StringFormat='Discovered: {0:MMM dd}'}"
                   FontSize="11"
                   Foreground="#64748b"/>
    </StackPanel>
</Border>
```

## Features

### Phase 1: Basic Kanban (MVP)
- [ ] Load issue da `.personal/active/tech-debt/`
- [ ] Kanban board con 4 colonne (open, in_progress, blocked, completed)
- [ ] Click card → mostra dettagli
- [ ] Drag & drop per cambiare status

### Phase 2: Editing
- [ ] Form per modificare frontmatter (status, priority, type)
- [ ] Quick save metadata
- [ ] Edit full document button → markdown editor

### Phase 3: Creation & Templates
- [ ] New issue button
- [ ] Template system (use `_TEMPLATE.md`)
- [ ] Auto-generate filename da title

### Phase 4: Advanced Features
- [ ] Filtering (type, priority, search)
- [ ] Sorting (priority, date)
- [ ] Archive completed issues
- [ ] Link to decisions/reports (navigate to related files)
- [ ] Statistics (count by type/priority)

## Integration Points

- **File Tree**: Right-click on `.personal/active/tech-debt/` → "Open Issue Board"
- **Tab System**: "Issues" tab accanto a .personal, docs, ecc.
- **Markdown Rendering**: Usa feature markdown per mostrare body
- **Terminal**: Possibile integrazione - Claude Code per creare issue

## Dependencies

```bash
dotnet add package YamlDotNet
# (Markdown.Avalonia già aggiunto in feature-markdown-rendering)
```

## Success Criteria

- [ ] Vedo tutte le issue in kanban board
- [ ] Posso cambiare status con drag & drop
- [ ] Posso modificare rapidamente priority/type
- [ ] Posso creare nuove issue da template
- [ ] File markdown vengono aggiornati correttamente (solo frontmatter)
- [ ] UI è fluida e responsive

## Open Questions

1. Supportare issue in progetti diversi (non solo SheetAtlas)?
2. Sync con GitHub Issues (future)?
3. Time tracking per issue?
4. Assignee field?
5. Comments/changelog su issue?

## Related

- **feature-markdown-rendering**: Prerequisito per rendering body
- **feature-embedded-terminal**: Possibile integrazione per creation assistita
