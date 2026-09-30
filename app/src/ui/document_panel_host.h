#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace dev_dash::services { class DocumentLoader; }

namespace dev_dash::ui
{
    class MarkdownRenderer;
    class StatusSink;

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

        // Where link and open failures are reported. Optional: the status
        // bar belongs to the shell, which comes and goes with the project.
        void SetStatusSink(StatusSink* status) { _status = status; }

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
        void DrainPendingOpens();
        void ReportError(std::string message);

        services::DocumentLoader& _loader;
        MarkdownRenderer& _renderer;
        StatusSink* _status = nullptr;
        std::vector<Panel> _panels;
        // Opened after the frame's panels are drawn: a click arrives while
        // _panels is being iterated.
        std::vector<std::filesystem::path> _pendingOpens;
        // Document being drawn, so relative links resolve against it.
        std::filesystem::path _currentDocument;
    };
}
