#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace dev_dash::services { class DocumentLoader; }

namespace dev_dash::ui
{
    class MarkdownRenderer;

    class DocumentPanelHost
    {
    public:
        DocumentPanelHost(services::DocumentLoader& loader,
                          MarkdownRenderer& renderer);
        ~DocumentPanelHost() = default;

        DocumentPanelHost(const DocumentPanelHost&)            = delete;
        DocumentPanelHost& operator=(const DocumentPanelHost&) = delete;

        // Idempotent: opening an already-open path is a no-op.
        void OpenPanel(const std::filesystem::path& path);

        // Called once per frame between ImGui::NewFrame() and ImGui::Render().
        // With a non-zero dockId, new documents dock there as tabs instead
        // of floating free.
        void Render(unsigned int dockId = 0);

    private:
        struct Panel
        {
            std::string title;
            std::filesystem::path path;
            std::string content;
            bool open = true;
        };

        void HandleLinkClick(std::string_view url);
        void DrainPendingImports();

        services::DocumentLoader& _loader;
        MarkdownRenderer& _renderer;
        std::vector<Panel> _panels;
        std::vector<std::filesystem::path> _pendingImports;
    };
}
