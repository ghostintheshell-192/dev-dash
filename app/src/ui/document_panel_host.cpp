#include "document_panel_host.h"
#include "markdown_renderer.h"
#include "status_sink.h"
#include "theme.h"
#include "../services/document_loader.h"

#include <algorithm>
#include <iostream>
#include <string_view>

#include <imgui.h>

namespace dev_dash::ui
{
    DocumentPanelHost::DocumentPanelHost(services::DocumentLoader& loader,
                                         MarkdownRenderer& renderer)
        : _loader(loader)
        , _renderer(renderer)
    {
        _renderer.SetLinkHandler([this](std::string_view url)
        {
            HandleLinkClick(url);
        });
    }

    void DocumentPanelHost::OpenPanel(const std::filesystem::path& path)
    {
        for (const auto& p : _panels)
            if (p.path == path)
                return;

        auto result = _loader.Load(path);
        if (result.content.empty())
        {
            ReportError("Could not open " + path.string());
            return;
        }

        const std::string title = path.filename().string() + "##" + path.string();
        _panels.push_back({title, path, std::move(result.content), true});
    }

    void DocumentPanelHost::Render(unsigned int dockId)
    {
        DrainPendingOpens();

        for (auto& panel : _panels)
        {
            if (!panel.open)
                continue;
            if (dockId != 0)
                ImGui::SetNextWindowDockID(dockId, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(700, 900), ImGuiCond_FirstUseEver);
            // Reading panes are airy: wider padding than the data-dense panels.
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                                CurrentTheme().readingPadding);
            if (ImGui::Begin(panel.title.c_str(), &panel.open))
            {
                _currentDocument = panel.path;
                _renderer.print(panel.content.data(), panel.content.data() + panel.content.size());
            }
            ImGui::End();
            ImGui::PopStyleVar();
        }

        std::erase_if(_panels, [](const Panel& p) { return !p.open; });
    }

    void DocumentPanelHost::HandleLinkClick(std::string_view url)
    {
        using Kind = services::LinkTarget::Kind;
        const services::LinkTarget target = _loader.ResolveLink(_currentDocument, url);

        switch (target.kind)
        {
        case Kind::kImport:
        case Kind::kDocument:
            _pendingOpens.push_back(target.path);
            break;
        case Kind::kFile:
            if (!MarkdownRenderer::OpenExternalUrl(services::DocumentLoader::ToFileUrl(target.path)))
                ReportError("Could not open " + target.path.string());
            break;
        case Kind::kExternal:
            if (!MarkdownRenderer::OpenExternalUrl(target.url))
                ReportError("Could not open " + target.url);
            break;
        case Kind::kAnchor:
            if (_status != nullptr)
                _status->Set(StatusSink::Level::kInfo,
                             "Links to a section (" + std::string(url) + ") are not followed yet");
            break;
        case Kind::kMissing:
            ReportError("Link target not found: " + target.path.string());
            break;
        }
    }

    void DocumentPanelHost::DrainPendingOpens()
    {
        for (const auto& path : _pendingOpens)
            OpenPanel(path);
        _pendingOpens.clear();
    }

    void DocumentPanelHost::ReportError(std::string message)
    {
        std::cerr << "[warn] DocumentPanelHost: " << message << '\n';
        if (_status != nullptr)
            _status->Set(StatusSink::Level::kError, std::move(message));
    }
}
