using Avalonia.Controls;
using Avalonia.Input;
using DevDash.ViewModels;

namespace DevDash.Views;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
    }

    private void OnProjectClicked(object? sender, PointerPressedEventArgs e)
    {
        if (sender is Border border && border.DataContext is ProjectViewModel project)
        {
            if (DataContext is MainWindowViewModel vm)
            {
                vm.SelectedProject = project;
            }
        }
    }

    private void OnWorkspaceRowClicked(object? sender, PointerPressedEventArgs e)
    {
        if (sender is Border border && border.DataContext is WorkspaceConfigViewModel workspace)
        {
            if (DataContext is MainWindowViewModel vm)
            {
                vm.SelectedConfiguredWorkspace = workspace;
            }
        }
    }
}
