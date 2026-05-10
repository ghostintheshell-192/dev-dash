#include "markdown_renderer.h"
#include "font_library.h"

#include <iostream>

#include <SDL3/SDL.h>
#include <imgui.h>

namespace dev_dash::ui
{
    MarkdownRenderer::MarkdownRenderer(const FontLibrary& fonts)
        : _fonts(fonts)
        , _linkHandler([](std::string_view url)
        {
            const std::string urlStr(url);
            if (!SDL_OpenURL(urlStr.c_str()))
                std::cerr << "[error] SDL_OpenURL failed for '" << urlStr
                          << "': " << SDL_GetError() << '\n';
        })
    {
    }

    void MarkdownRenderer::SetLinkHandler(LinkHandler handler)
    {
        _linkHandler = std::move(handler);
    }

    ImFont* MarkdownRenderer::get_font() const
    {
        switch (m_hlevel)
        {
        case 1: return _fonts.BoldH1();
        case 2: return _fonts.BoldH2();
        case 3:
        case 4:
        case 5:
        case 6: return _fonts.BoldH3();
        default: break;
        }

        if (m_is_strong) return _fonts.Bold();
        if (m_is_em)     return _fonts.Italic();

        return nullptr;
    }

    void MarkdownRenderer::open_url() const
    {
        if (_linkHandler)
            _linkHandler(m_href);
    }

    void MarkdownRenderer::SPAN_CODE(bool e)
    {
        if (e)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.40f, 1.0f));
        else
            ImGui::PopStyleColor();
    }

    void MarkdownRenderer::BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool e)
    {
        m_is_code = e;
        if (e)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.40f, 1.0f));
        else
            ImGui::PopStyleColor();
    }
}
