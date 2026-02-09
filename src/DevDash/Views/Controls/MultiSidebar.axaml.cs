using System.Collections.ObjectModel;
using Avalonia;
using Avalonia.Controls;
using CommunityToolkit.Mvvm.Input;
using DevDash.Models;

namespace DevDash.Views.Controls;

public partial class MultiSidebar : UserControl
{
    public static readonly StyledProperty<ObservableCollection<SidebarItem>> ItemsProperty =
        AvaloniaProperty.Register<MultiSidebar, ObservableCollection<SidebarItem>>(
            nameof(Items));

    public static readonly StyledProperty<bool> AllowMultipleOpenProperty =
        AvaloniaProperty.Register<MultiSidebar, bool>(
            nameof(AllowMultipleOpen), defaultValue: false);

    public static readonly StyledProperty<object?> ParentDataContextProperty =
        AvaloniaProperty.Register<MultiSidebar, object?>(
            nameof(ParentDataContext));

    public ObservableCollection<SidebarItem> Items
    {
        get => GetValue(ItemsProperty);
        set => SetValue(ItemsProperty, value);
    }

    public bool AllowMultipleOpen
    {
        get => GetValue(AllowMultipleOpenProperty);
        set => SetValue(AllowMultipleOpenProperty, value);
    }

    public object? ParentDataContext
    {
        get => GetValue(ParentDataContextProperty);
        set => SetValue(ParentDataContextProperty, value);
    }

    public IRelayCommand<SidebarItem> ToggleSidebarCommand { get; }

    public MultiSidebar()
    {
        Items = new ObservableCollection<SidebarItem>();
        ToggleSidebarCommand = new RelayCommand<SidebarItem>(ToggleSidebar);

        InitializeComponent();
    }

    private void ToggleSidebar(SidebarItem? item)
    {
        if (item == null) return;

        // Se la sidebar è già aperta, la chiudo
        if (item.IsOpen)
        {
            item.IsOpen = false;
        }
        else
        {
            // Se non consento aperture multiple, chiudo tutte le altre
            if (!AllowMultipleOpen)
            {
                foreach (var sidebar in Items)
                {
                    if (sidebar != item)
                    {
                        sidebar.IsOpen = false;
                    }
                }
            }

            // Apro la sidebar cliccata
            item.IsOpen = true;
        }
    }
}
