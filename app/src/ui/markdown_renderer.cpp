#include "markdown_renderer.h"
#include "font_library.h"
#include "theme.h"

#include <iostream>

#include <SDL3/SDL.h>
#include <imgui.h>

namespace dev_dash::ui
{
    MarkdownRenderer::MarkdownRenderer(const FontLibrary& fonts)
        : _fonts(fonts)
        , _linkHandler([](std::string_view url) { OpenExternalUrl(url); })
    {
    }

    bool MarkdownRenderer::OpenExternalUrl(std::string_view url)
    {
        const std::string urlStr(url);
        if (SDL_OpenURL(urlStr.c_str()))
            return true;
        std::cerr << "[error] SDL_OpenURL failed for '" << urlStr
                  << "': " << SDL_GetError() << '\n';
        return false;
    }

    void MarkdownRenderer::SetLinkHandler(LinkHandler handler)
    {
        _linkHandler = std::move(handler);
    }

    void MarkdownRenderer::get_font(font_info& info) const
    {
        info.font = nullptr;
        info.size = 0.0f;

        switch (m_hlevel)
        {
        case 1: info.font = _fonts.BoldH1(); return;
        case 2: info.font = _fonts.BoldH2(); return;
        case 3:
        case 4:
        case 5:
        case 6: info.font = _fonts.BoldH3(); return;
        default: break;
        }

        if (m_is_strong) { info.font = _fonts.Bold();   return; }
        if (m_is_em)     { info.font = _fonts.Italic();  return; }
    }

    void MarkdownRenderer::open_url() const
    {
        if (_linkHandler)
            _linkHandler(m_href);
    }

    void MarkdownRenderer::SPAN_CODE(bool e)
    {
        if (e)
            ImGui::PushStyleColor(ImGuiCol_Text, CurrentTheme().accentHover);
        else
            ImGui::PopStyleColor();
    }

    void MarkdownRenderer::BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool e)
    {
        m_is_code = e;
        if (e)
            ImGui::PushStyleColor(ImGuiCol_Text, CurrentTheme().accentHover);
        else
            ImGui::PopStyleColor();
    }
}
