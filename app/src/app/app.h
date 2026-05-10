#pragma once

#include <filesystem>
#include <memory>

namespace dev_dash::platform
{
    class SdlSession;
    class Window;
    class VulkanContext;
    class Swapchain;
    class FrameResources;
    class ImGuiBackend;
}

namespace dev_dash::ui
{
    class FontLibrary;
    class MarkdownRenderer;
    class DocumentPanelHost;
    class ProjectSelectorPanel;
    class EffectiveConfigPanel;
    class ScaffoldDiffPanel;
}

namespace dev_dash::services
{
    class DocumentLoader;
    class ConfigResolver;
    class ScaffoldRepository;
    class DiffEngine;
    class PromoteEngine;
}

namespace dev_dash::app
{
    enum class AppState { kSelectingProject, kViewingConfig, kScaffoldDiff };

    class App
    {
    public:
        App();
        ~App();

        App(const App&)            = delete;
        App& operator=(const App&) = delete;

        int Run();

    private:
        bool Init();
        bool MainLoop();
        void OnProjectSelected(const std::filesystem::path& path);

        // ----- Construction order (in Init body) -----
        // 1. _sdlSession
        // 2. _window
        // 3. _vulkanContext (needs window)
        // 4. _swapchain (needs context + window)
        // 5. _frameResources (needs context + swapchain)
        // 6. ImGui::CreateContext()  -- raw call, paired with DestroyContext in ~App
        // 7. _fonts                  -- populates atlas
        // 8. _imguiBackend (needs window + context + swapchain; builds font texture)
        // 9. _documentLoader
        // 10. _markdownRenderer (needs fonts)
        // 11. _documentPanelHost (needs loader + renderer; registers LinkHandler)
        // 12. _configResolver, _diffEngine, _scaffoldRepository
        // 13. _projectSelectorPanel (needs window handle)
        //
        // ----- Declaration order (drives LIFO destruction) -----
        // Last declared = first destroyed.
        // UI panels declared last: they hold refs to services declared earlier.

        std::unique_ptr<platform::SdlSession>     _sdlSession;
        std::unique_ptr<platform::Window>         _window;
        std::unique_ptr<platform::VulkanContext>  _vulkanContext;
        std::unique_ptr<platform::Swapchain>      _swapchain;
        std::unique_ptr<platform::FrameResources> _frameResources;
        std::unique_ptr<platform::ImGuiBackend>   _imguiBackend;
        std::unique_ptr<ui::FontLibrary>          _fonts;
        std::unique_ptr<services::DocumentLoader> _documentLoader;

        // _documentPanelHost before _markdownRenderer: renderer destroyed first,
        // releasing the LinkHandler lambda before the host that owns the panels.
        std::unique_ptr<ui::DocumentPanelHost>    _documentPanelHost;
        std::unique_ptr<ui::MarkdownRenderer>     _markdownRenderer;

        std::unique_ptr<services::ConfigResolver>     _configResolver;
        std::unique_ptr<services::ScaffoldRepository> _scaffoldRepository;
        std::unique_ptr<services::DiffEngine>         _diffEngine;
        std::unique_ptr<services::PromoteEngine>      _promoteEngine;
        std::unique_ptr<ui::ProjectSelectorPanel>    _projectSelectorPanel;
        // Panels declared last → destroyed first, before the services they reference.
        std::unique_ptr<ui::EffectiveConfigPanel>    _effectiveConfigPanel;
        std::unique_ptr<ui::ScaffoldDiffPanel>       _scaffoldDiffPanel;

        AppState _appState = AppState::kSelectingProject;
    };
}
