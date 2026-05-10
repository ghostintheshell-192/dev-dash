#include "document_panel_host.h"
#include "markdown_renderer.h"
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
            std::cerr << "[warn] DocumentPanelHost: could not open: " << path << '\n';
            return;
        }

        const std::string title = path.filename().string() + "##" + path.string();
        _panels.push_back({title, path, std::move(result.content), true});
    }

    void DocumentPanelHost::Render()
    {
        DrainPendingImports();

        for (auto& panel : _panels)
        {
            if (!panel.open)
                continue;
            ImGui::SetNextWindowSize(ImVec2(700, 900), ImGuiCond_FirstUseEver);
            if (ImGui::Begin(panel.title.c_str(), &panel.open))
                _renderer.print(panel.content.data(), panel.content.data() + panel.content.size());
            ImGui::End();
        }

        std::erase_if(_panels, [](const Panel& p) { return !p.open; });
    }

    void DocumentPanelHost::HandleLinkClick(std::string_view url)
    {
        constexpr std::string_view kPrefix = "claudeimport://";
        if (url.starts_with(kPrefix))
            _pendingImports.emplace_back(url.substr(kPrefix.size()));
    }

    void DocumentPanelHost::DrainPendingImports()
    {
        for (const auto& path : _pendingImports)
            OpenPanel(path);
        _pendingImports.clear();
    }
}
