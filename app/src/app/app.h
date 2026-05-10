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
}

namespace dev_dash::services
{
    class DocumentLoader;
    class ConfigResolver;
}

namespace dev_dash::app
{
    enum class AppState { kSelectingProject, kViewingConfig };

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
        // 12. _configResolver
        // 13. _projectSelectorPanel (needs window handle)
        //
        // ----- Declaration order (drives LIFO destruction) -----
        // Last declared = first destroyed.
        // _effectiveConfigPanel holds refs to _configResolver + _documentPanelHost
        // → must be declared AFTER both (destroyed before both).

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

        std::unique_ptr<services::ConfigResolver>  _configResolver;
        std::unique_ptr<ui::ProjectSelectorPanel>  _projectSelectorPanel;
        // _effectiveConfigPanel last → destroyed first, before _configResolver
        // and _documentPanelHost whose references it holds.
        std::unique_ptr<ui::EffectiveConfigPanel>  _effectiveConfigPanel;

        AppState _appState = AppState::kSelectingProject;
    };
}
