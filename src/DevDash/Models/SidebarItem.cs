using Avalonia.Controls.Templates;
using CommunityToolkit.Mvvm.ComponentModel;

namespace DevDash.Models;

public partial class SidebarItem : ObservableObject
{
    [ObservableProperty]
    private string _iconData = string.Empty;

    [ObservableProperty]
    private string _tooltip = string.Empty;

    [ObservableProperty]
    private IDataTemplate? _contentTemplate;

    [ObservableProperty]
    private bool _isOpen;

    [ObservableProperty]
    private double _width = 256;
}
