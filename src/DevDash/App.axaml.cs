using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Data.Core;
using Avalonia.Data.Core.Plugins;
using System.Linq;
using Avalonia.Markup.Xaml;
using DevDash.Services;
using DevDash.ViewModels;
using DevDash.Views;

namespace DevDash;

public partial class App : Application
{
    public override void Initialize()
    {
        AvaloniaXamlLoader.Load(this);
    }

    public override void OnFrameworkInitializationCompleted()
    {
        if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
        {
            // Avoid duplicate validations from both Avalonia and the CommunityToolkit.
            DisableAvaloniaDataAnnotationValidation();

            // Setup services (simple DI without container)
            var fileSystemService = new FileSystemService();
            var appSettingsService = new AppSettingsService();
            var workspaceService = new WorkspaceService(fileSystemService, appSettingsService);
            var configurationService = new ConfigurationService(fileSystemService);
            var scaffoldService = new ScaffoldService();

            // Create ViewModel with dependencies
            var viewModel = new MainWindowViewModel(
                workspaceService,
                fileSystemService,
                configurationService,
                appSettingsService,
                scaffoldService);

            // Create main window
            var mainWindow = new MainWindow
            {
                DataContext = viewModel,
            };

            // Provide StorageProvider to ViewModel for folder picker dialogs
            viewModel.StorageProvider = mainWindow.StorageProvider;

            // Initialize data
            _ = viewModel.InitializeAsync();

            desktop.MainWindow = mainWindow;
        }

        base.OnFrameworkInitializationCompleted();
    }

    private void DisableAvaloniaDataAnnotationValidation()
    {
        // Get an array of plugins to remove
        var dataValidationPluginsToRemove =
            BindingPlugins.DataValidators.OfType<DataAnnotationsValidationPlugin>().ToArray();

        // remove each entry found
        foreach (var plugin in dataValidationPluginsToRemove)
        {
            BindingPlugins.DataValidators.Remove(plugin);
        }
    }
}
