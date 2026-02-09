using System;
using System.Collections.ObjectModel;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using DevDash.ViewModels;

namespace DevDash.Views;

public partial class FileTreeView : UserControl
{
    public static readonly StyledProperty<ObservableCollection<FileTreeItemViewModel>?> ItemsSourceProperty =
        AvaloniaProperty.Register<FileTreeView, ObservableCollection<FileTreeItemViewModel>?>(nameof(ItemsSource));

    public static readonly StyledProperty<FileTreeItemViewModel?> SelectedItemProperty =
        AvaloniaProperty.Register<FileTreeView, FileTreeItemViewModel?>(nameof(SelectedItem),
            defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);

    public ObservableCollection<FileTreeItemViewModel>? ItemsSource
    {
        get => GetValue(ItemsSourceProperty);
        set => SetValue(ItemsSourceProperty, value);
    }

    public FileTreeItemViewModel? SelectedItem
    {
        get => GetValue(SelectedItemProperty);
        set => SetValue(SelectedItemProperty, value);
    }

    public FileTreeView()
    {
        InitializeComponent();
    }

    private void OnTreeSelectionChanged(object? sender, SelectionChangedEventArgs e)
    {
        if (sender is TreeView tv && tv.SelectedItem is FileTreeItemViewModel item)
        {
            SelectedItem = item;
        }
    }
}
