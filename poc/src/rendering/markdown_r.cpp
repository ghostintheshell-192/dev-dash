#include <SDL3/SDL.h>
#include <string_view>
#include <iostream>
#include "markdown_r.h"

namespace Rendering
{
    MarkdownRenderer::MarkdownRenderer(std::vector<std::string> &pendingPanels) : _pendingPanels(pendingPanels) {
    }

    void MarkdownRenderer::InitFonts()
    {
        std::string fontsDir;
        if (const char *base = SDL_GetBasePath())
            fontsDir = base;
        fontsDir += "assets/fonts/";

        ImFontAtlas *atlas = ImGui::GetIO().Fonts;
        constexpr float kBodySize = 16.0f;

        // IBM Plex Sans covers Latin but omits many Unicode symbols (arrows,
        // dingbats, etc.). DejaVu Sans is merged in after each IBM Plex face
        // to supply the missing glyphs. MergeMode appends into the last-added
        // font, so the merge call must immediately follow its primary face.
        static constexpr ImWchar kPrimaryRanges[] = {
            0x0020,
            0x00FF, // Basic Latin + Latin Supplement
            0x2000,
            0x206F, // General Punctuation  (—  …  •  etc.)
            0x2190,
            0x21FF, // Arrows               (→  ←  etc.)
            0x2700,
            0x27BF, // Dingbats             (✓  ✗  etc.)
            0,
        };
        static constexpr ImWchar kFallbackRanges[] = {
            0x0100,
            0x024F, // Latin Extended
            0x2000,
            0x27BF, // Punctuation + Arrows + Dingbats (all in one block)
            0,
        };
        constexpr const char *kDejaVu = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";

        auto load = [&](const char *file, float size) -> ImFont *
        {
            ImFont *f = atlas->AddFontFromFileTTF((fontsDir + file).c_str(), size,
                                                  nullptr, kPrimaryRanges);
            if (!f)
            {
                std::cerr << "[warn] Font not found: " << fontsDir << file << '\n';
                return f;
            }
            ImFontConfig merge;
            merge.MergeMode = true;
            atlas->AddFontFromFileTTF(kDejaVu, size, &merge, kFallbackRanges);
            return f;
        };

        _fonts.regular = load("IBMPlexSans-Regular.ttf", kBodySize);
        _fonts.italic = load("IBMPlexSans-Italic.ttf", kBodySize);
        _fonts.bold = load("IBMPlexSans-Bold.ttf", kBodySize);
        _fonts.boldH1 = load("IBMPlexSans-Bold.ttf", 30.0f);
        _fonts.boldH2 = load("IBMPlexSans-Bold.ttf", 22.5f);
        _fonts.boldH3 = load("IBMPlexSans-Bold.ttf", 17.55f);

    }

    ImFont *Rendering::MarkdownRenderer::get_font() const
    {
        // Decide in base allo stato corrente:
        // - m_hlevel: 1, 2, 3+ → H1, H2, H3 font
        // - m_is_strong: bold
        // - m_is_em: italic
        // - altrimenti: nullptr (= default ImGui font)

        switch (m_hlevel)
        {
        case 1:
            return _fonts.boldH1;
        case 2:
            return _fonts.boldH2;
        case 3:
            return _fonts.boldH3;
        case 4:
            return _fonts.boldH3;
        case 5:
            return _fonts.boldH3;
        case 6:
            return _fonts.boldH3;
        default:
            break;
        }

        if (m_is_strong)
            return _fonts.bold;
        if (m_is_em)
            return _fonts.italic;

        return nullptr;
    }

    void Rendering::MarkdownRenderer::open_url() const
    {
        constexpr std::string_view kClaudeImportPrefix = "claudeimport://";
        if (m_href.starts_with(kClaudeImportPrefix))
        {
            _pendingPanels.push_back(m_href.substr(kClaudeImportPrefix.size()));
        }
        else if (!SDL_OpenURL(m_href.c_str()))
        {
            std::cerr << "[error] SDL_OpenURL failed for '" << m_href
                      << "': " << SDL_GetError() << '\n';
        }
    }

    bool Rendering::MarkdownRenderer::get_image(image_info &) const
    {
        return false; // niente immagini per ora
    }

    void Rendering::MarkdownRenderer::SPAN_CODE(bool e)
    {
        if (e)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.40f, 1.0f));
        else
            ImGui::PopStyleColor();
    }

    void Rendering::MarkdownRenderer::BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool e)
    {
        m_is_code = e;

        if (e)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.40f, 1.0f));
        else
            ImGui::PopStyleColor();
    }
}
