using System.Collections.ObjectModel;
using CommunityToolkit.Mvvm.ComponentModel;
using DevDash.Models;

namespace DevDash.ViewModels;

/// <summary>
/// ViewModel per un item nell'albero file (.personal)
/// </summary>
public partial class FileTreeItemViewModel : ViewModelBase
{
    [ObservableProperty]
    private bool _isExpanded;

    [ObservableProperty]
    private bool _isSelected;

    public PersonalFile File { get; }
    public ObservableCollection<FileTreeItemViewModel> Children { get; } = [];

    public string Name => File.Name;
    public string Path => File.Path;
    public bool IsFolder => File.IsFolder;
    public bool IsFile => File.IsFile;
    public Priority Priority => File.Priority;

    public FileTreeItemViewModel(PersonalFile file)
    {
        File = file;

        if (file.Children != null)
        {
            foreach (var child in file.Children)
            {
                Children.Add(new FileTreeItemViewModel(child));
            }
        }

        // Espandi le cartelle di primo livello
        if (IsFolder && (Name == "active" || Name == "specs" || Name == "reference"))
        {
            IsExpanded = true;
        }
    }
}
