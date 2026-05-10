#pragma once

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
}

namespace dev_dash::services
{
    class DocumentLoader;
}

namespace dev_dash::app
{
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
        //
        // ----- Declaration order (drives LIFO destruction) -----
        // Reverse the callback dependency: renderer must die before its host.

        std::unique_ptr<platform::SdlSession>     _sdlSession;
        std::unique_ptr<platform::Window>         _window;
        std::unique_ptr<platform::VulkanContext>  _vulkanContext;
        std::unique_ptr<platform::Swapchain>      _swapchain;
        std::unique_ptr<platform::FrameResources> _frameResources;
        std::unique_ptr<platform::ImGuiBackend>   _imguiBackend;
        std::unique_ptr<ui::FontLibrary>          _fonts;
        std::unique_ptr<services::DocumentLoader> _documentLoader;

        // CRITICAL: _documentPanelHost MUST be declared before _markdownRenderer.
        // The host registers a LinkHandler lambda capturing [this] on the renderer.
        // C++ destroys members in REVERSE declaration order, so _markdownRenderer
        // is destroyed first → its LinkHandler (capturing host) is released →
        // _documentPanelHost destroyed safely after.
        std::unique_ptr<ui::DocumentPanelHost>    _documentPanelHost;
        std::unique_ptr<ui::MarkdownRenderer>     _markdownRenderer;
    };
}
