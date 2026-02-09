---
type: feature
priority: high
status: planned
category: ui
related: [feature-issue-management]
---

# Markdown Rendering & Editing

## Overview

Visualizzare e modificare file markdown con rendering live, supportando il workflow di documentazione e issue tracking in `.personal/`.

## User Story

Come utente, voglio:
- Vedere file markdown renderizzati (non raw text)
- Modificare markdown con preview live
- Passare facilmente tra view/edit mode
- Vedere syntax highlighting nel codice embedded
- Navigare tra link interni (altri file .md)

## Use Cases

1. **Leggere documentazione**: File in `.personal/reference/`, specs, ecc.
2. **Editare issue**: Modificare status, priority, aggiungere note
3. **Scrivere planning docs**: Roadmap, design docs
4. **Browse decisions**: ADR in `.personal/reference/decisions/`

## Technical Approach

### Rendering: Avalonia.Markdown

**Library:**
```bash
dotnet add package Markdown.Avalonia
```

**Basic Implementation:**
```csharp
// In XAML
<mdxam:MarkdownScrollViewer
    Markdown="{Binding SelectedFileContent}"
    MarkdownStyleName="Standard"/>
```

**Features:**
- Supporto GFM (GitHub Flavored Markdown)
- Syntax highlighting per code blocks
- Tabelle, task lists, strikethrough
- Link support (navigazione)
- Immagini embedded

### Editing: Toggle View Approach (MVP)

**UI Flow:**

```
┌─────────────────────────────────────┐
│ [Preview] [Edit] [Save]             │ ← Mode selector
├─────────────────────────────────────┤
│                                     │
│ View Mode (default):                │
│ ┌─────────────────────────────┐   │
│ │ # Title                     │   │
│ │                             │   │
│ │ Rendered markdown...        │   │
│ │                             │   │
│ └─────────────────────────────┘   │
│                                     │
│ Edit Mode:                          │
│ ┌─────────────────────────────┐   │
│ │ # Title                     │   │
│ │                             │   │
│ │ Raw markdown text...        │   │
│ │ [Editing...]                │   │
│ └─────────────────────────────┘   │
└─────────────────────────────────────┘
```

**Implementation:**
```csharp
public class FileContentViewModel : ViewModelBase
{
    [ObservableProperty]
    private string _markdownContent;

    [ObservableProperty]
    private bool _isEditMode;

    [ObservableProperty]
    private string _editableContent;

    [RelayCommand]
    private void ToggleEditMode()
    {
        if (IsEditMode)
        {
            // Switching to view → save changes
            MarkdownContent = EditableContent;
            SaveToFile();
        }
        else
        {
            // Switching to edit → copy current content
            EditableContent = MarkdownContent;
        }
        IsEditMode = !IsEditMode;
    }
}
```

**XAML:**
```xml
<Grid>
    <!-- View Mode -->
    <mdxam:MarkdownScrollViewer
        Markdown="{Binding MarkdownContent}"
        IsVisible="{Binding !IsEditMode}"/>

    <!-- Edit Mode -->
    <TextBox Text="{Binding EditableContent}"
             FontFamily="Consolas,monospace"
             AcceptsReturn="True"
             IsVisible="{Binding IsEditMode}"/>

    <!-- Toolbar -->
    <StackPanel Orientation="Horizontal">
        <Button Command="{Binding ToggleEditModeCommand}">
            <TextBlock Text="{Binding IsEditMode,
                       Converter={StaticResource BoolToEditText}}"/>
        </Button>
    </StackPanel>
</Grid>
```

### Advanced: Split View (Phase 2)

```
┌──────────────────┬──────────────────┐
│ # Title          │ Title            │
│                  │                  │
│ ## Section       │ Section          │
│ - Item 1         │ • Item 1         │
│ - Item 2         │ • Item 2         │
│                  │                  │
│ [Raw Markdown]   │ [Live Preview]   │
└──────────────────┴──────────────────┘
```

**Pros:**
- Vedi preview mentre editi
- Feedback immediato

**Cons:**
- Richiede più spazio schermo
- Più complesso da implementare (sync scroll)

### Alternative: External Editor Button

**Quick Win:**
```csharp
[RelayCommand]
private void OpenInExternalEditor()
{
    Process.Start(new ProcessStartInfo
    {
        FileName = "code",  // VS Code
        Arguments = _currentFilePath,
        UseShellExecute = true
    });
}
```

**Pros:** Zero implementazione editing
**Cons:** Esce dall'app, context switch

## Frontmatter Support (Issue Management Integration)

Per file con frontmatter YAML (issue, specs):

```csharp
public class MarkdownFile
{
    public Dictionary<string, object> Frontmatter { get; set; }
    public string Body { get; set; }

    public static MarkdownFile Parse(string content)
    {
        var match = Regex.Match(content, @"^---\s*\n(.*?)\n---\s*\n(.*)$",
                                RegexOptions.Singleline);

        if (match.Success)
        {
            var yamlDeserializer = new DeserializerBuilder().Build();
            var frontmatter = yamlDeserializer
                .Deserialize<Dictionary<string, object>>(match.Groups[1].Value);

            return new MarkdownFile
            {
                Frontmatter = frontmatter,
                Body = match.Groups[2].Value
            };
        }

        return new MarkdownFile { Body = content };
    }
}
```

**UI per Issue Files:**
```
┌─────────────────────────────────────┐
│ Type: [performance ▼]  Priority: [medium ▼] │
│ Status: [open ▼]  Discovered: 2025-12-11    │
│ [Quick Save Metadata]               │
├─────────────────────────────────────┤
│ # Issue Title                       │
│                                     │
│ ## Problem                          │
│ Rendered markdown body...           │
│                                     │
│ [Edit Full Document]                │
└─────────────────────────────────────┘
```

## Implementation Phases

### Phase 1: Basic Rendering (MVP)
**Timeline: 1 giorno**

- [x] Add Markdown.Avalonia package
- [ ] Display markdown files in main content area
- [ ] Basic navigation (click file → show rendered)
- [ ] Scroll support

### Phase 2: Toggle Edit Mode
**Timeline: 1-2 giorni**

- [ ] Edit button → switch to TextBox
- [ ] Save changes to file
- [ ] Unsaved changes warning
- [ ] Keyboard shortcuts (Ctrl+E for edit, Ctrl+S for save)

### Phase 3: Frontmatter Editing
**Timeline: 2 giorni**

- [ ] Add YamlDotNet package
- [ ] Parse frontmatter
- [ ] Show metadata form for issue files
- [ ] Quick edit status/priority
- [ ] Save only frontmatter without touching body

### Phase 4: Enhanced UX (Future)
**Timeline: 3-4 giorni**

- [ ] Split view edit mode
- [ ] Sync scroll between editor and preview
- [ ] Markdown toolbar (bold, italic, lists, ecc.)
- [ ] Live preview while typing
- [ ] Link navigation (Ctrl+Click on links)
- [ ] Search in document

## Dependencies

```bash
# Phase 1
dotnet add package Markdown.Avalonia

# Phase 3
dotnet add package YamlDotNet

# Phase 4 (optional)
dotnet add package AvaloniaEdit  # For advanced editing
```

## UI Integration

**File Tree Integration:**
```csharp
// When user clicks on .md file
private void OnFileSelected(PersonalFile file)
{
    if (file.Extension == ".md")
    {
        var content = File.ReadAllText(file.Path);
        var mdFile = MarkdownFile.Parse(content);

        // Show rendered markdown
        SelectedFileContent = mdFile.Body;

        // Show metadata panel if frontmatter exists
        if (mdFile.Frontmatter != null)
        {
            ShowMetadataPanel = true;
            FileMetadata = mdFile.Frontmatter;
        }
    }
}
```

**Tab System:**
- `.personal` → rendered markdown
- `docs` → rendered markdown
- `issues` → metadata form + rendered body
- `Config` → raw JSON/YAML (different view)

## Styling

**Custom Markdown Theme:**
```xaml
<mdxam:MarkdownScrollViewer.Styles>
    <Style Selector="mdxam|MarkdownScrollViewer">
        <Setter Property="Foreground" Value="#e2e8f0"/>
        <Setter Property="Background" Value="#0f172a"/>
    </Style>
    <Style Selector="mdxam|MarkdownScrollViewer Code">
        <Setter Property="Foreground" Value="#fbbf24"/>
        <Setter Property="Background" Value="#1e293b"/>
    </Style>
</mdxam:MarkdownScrollViewer.Styles>
```

Match DevDash dark theme.

## Open Questions

1. Auto-save vs manual save? (preferenza: manual con Ctrl+S)
2. Supportare template per nuovi file? (es. new issue → usa template)
3. Diff view per vedere modifiche prima di salvare?
4. Undo/redo stack?
5. Spell checking per markdown?

## Success Criteria

- [ ] File .md mostrati come rendered markdown (non raw)
- [ ] Posso editare e salvare modifiche
- [ ] Frontmatter YAML viene parsato e mostrato in form
- [ ] Posso modificare rapidamente status di issue
- [ ] Styling consistente con dark theme di DevDash
- [ ] Performance accettabile (render < 100ms per file tipici)

## Related Features

- **Issue Management**: Questa feature è prerequisito
- **Embedded Terminal**: Possibile integrazione (Claude Code può generare/modificare markdown)
- **Search**: Cercare nel contenuto renderizzato

## Security Considerations

- **No HTML rendering**: Solo markdown safe (prevent XSS)
- **File path validation**: Non permettere escape da `.personal/`
- **Link validation**: Solo link relativi o whitelisted domains

## Performance Considerations

- **Lazy rendering**: Renderizza solo file visibile
- **Cache**: Cache del rendering per file non modificati
- **Large files**: Warning/pagination per file > 1MB
- **Real-time preview**: Debounce per evitare re-render continui (300ms)

## Testing Strategy

**Unit Tests:**
- Frontmatter parsing
- File save/load
- Metadata extraction

**Integration Tests:**
- Render diversi markdown format (GFM, tables, code, ecc.)
- Edit → Save → Reload consistency
- Navigation tra file

**Manual Testing:**
- Rendering quality check
- Edit UX fluidity
- Theme consistency
