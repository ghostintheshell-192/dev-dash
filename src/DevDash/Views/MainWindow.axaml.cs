using System;
using System.ComponentModel;
using System.Windows.Input;
using Avalonia.Controls;
using Avalonia.Input;
using DevDash.ViewModels;

namespace DevDash.Views;

public partial class MainWindow : Window
{
    private MainWindowViewModel? _viewModel;
    private DocumentTabViewModel? _currentDocument;

    public MainWindow()
    {
        InitializeComponent();

        DataContextChanged += OnDataContextChanged;
        Loaded += OnLoaded;
    }

    private void OnDataContextChanged(object? sender, EventArgs e)
    {
        // Unsubscribe from old ViewModel
        if (_viewModel != null)
        {
            _viewModel.PropertyChanged -= OnViewModelPropertyChanged;
        }

        // Subscribe to new ViewModel
        _viewModel = DataContext as MainWindowViewModel;
        if (_viewModel != null)
        {
            _viewModel.PropertyChanged += OnViewModelPropertyChanged;
            UpdateMarkdownContent(_viewModel.SelectedDocument);
        }
    }

    private void OnViewModelPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        if (e.PropertyName == nameof(MainWindowViewModel.SelectedDocument))
        {
            UpdateMarkdownContent(_viewModel?.SelectedDocument);
        }
    }

    private void UpdateMarkdownContent(DocumentTabViewModel? doc)
    {
        // Unsubscribe from old document
        if (_currentDocument != null)
        {
            _currentDocument.PropertyChanged -= OnDocumentPropertyChanged;
        }

        _currentDocument = doc;

        // Subscribe to new document
        if (_currentDocument != null)
        {
            _currentDocument.PropertyChanged += OnDocumentPropertyChanged;
        }

        // Update the viewer
        if (MarkdownViewer != null)
        {
            MarkdownViewer.Markdown = doc?.Content ?? string.Empty;
        }
    }

    private void OnDocumentPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        if (e.PropertyName == nameof(DocumentTabViewModel.Content) ||
            e.PropertyName == nameof(DocumentTabViewModel.RawContent))
        {
            if (MarkdownViewer != null && _currentDocument != null)
            {
                MarkdownViewer.Markdown = _currentDocument.Content;
            }
        }
    }

    private void OnLoaded(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
    {
        // Configure hyperlink command
        if (MarkdownViewer?.Engine is Markdown.Avalonia.Markdown mdEngine)
        {
            mdEngine.HyperlinkCommand = new MarkdownLinkCommand(this);
        }

        // Initial content update (in case DataContextChanged fired before MarkdownViewer was ready)
        if (_viewModel != null)
        {
            UpdateMarkdownContent(_viewModel.SelectedDocument);
        }
    }

    private class MarkdownLinkCommand : ICommand
    {
        private readonly MainWindow _window;

        public MarkdownLinkCommand(MainWindow window)
        {
            _window = window;
        }

        public event EventHandler? CanExecuteChanged;

        public bool CanExecute(object? parameter) => true;

        public void Execute(object? parameter)
        {
            if (parameter is string url && _window.DataContext is MainWindowViewModel vm)
            {
                vm.HandleMarkdownLinkCommand.Execute(url);
            }
        }
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

    private void OnDocumentTabClicked(object? sender, PointerPressedEventArgs e)
    {
        if (sender is Border border && border.DataContext is DocumentTabViewModel doc)
        {
            if (DataContext is MainWindowViewModel vm)
            {
                vm.SelectedDocument = doc;
            }
        }
    }

    private void OnTechDebtItemClicked(object? sender, PointerPressedEventArgs e)
    {
        if (sender is Border border && border.DataContext is TechDebtItemViewModel item)
        {
            if (DataContext is MainWindowViewModel vm)
            {
                vm.OpenDocumentByPathCommand.Execute(item.FullPath);
            }
        }
    }

    private void OnAdrItemClicked(object? sender, PointerPressedEventArgs e)
    {
        if (sender is Border border && border.DataContext is AdrItemViewModel item)
        {
            if (DataContext is MainWindowViewModel vm)
            {
                vm.OpenDocumentByPathCommand.Execute(item.FullPath);
            }
        }
    }

    private void OnSpecItemClicked(object? sender, PointerPressedEventArgs e)
    {
        if (sender is Border border && border.DataContext is SpecItemViewModel item)
        {
            if (DataContext is MainWindowViewModel vm)
            {
                vm.OpenDocumentByPathCommand.Execute(item.FullPath);
            }
        }
    }
}
