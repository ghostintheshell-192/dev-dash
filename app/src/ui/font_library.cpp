#include "font_library.h"

#include <iostream>

#include <imgui.h>

namespace dev_dash::ui
{
    FontLibrary::FontLibrary(float uiScale, std::filesystem::path fontsDir)
    {
        ImFontAtlas* atlas = ImGui::GetIO().Fonts;
        const float kBodySize = 17.0f * uiScale;

        static constexpr ImWchar kPrimaryRanges[] = {
            0x0020, 0x00FF,
            0x2000, 0x206F,
            0x2190, 0x21FF,
            0x2700, 0x27BF,
            0,
        };
        static constexpr ImWchar kFallbackRanges[] = {
            0x0100, 0x024F,
            0x2000, 0x27BF,
            0,
        };

        // DejaVu Sans is bundled alongside the primary fonts (see assets/fonts/)
        // and merged in as a fallback for glyphs IBM Plex lacks.
        const std::filesystem::path dejaVu = fontsDir / "DejaVuSans.ttf";

        auto load = [&](const char* file, float size) -> ImFont*
        {
            const std::filesystem::path path = fontsDir / file;
            ImFont* f = atlas->AddFontFromFileTTF(
                path.string().c_str(), size, nullptr, kPrimaryRanges);
            if (!f)
            {
                std::cerr << "[warn] Font not found: " << path << '\n';
                return f;
            }
            ImFontConfig merge;
            merge.MergeMode = true;
            atlas->AddFontFromFileTTF(
                dejaVu.string().c_str(), size, &merge, kFallbackRanges);
            return f;
        };

        _regular = load("IBMPlexSans-Regular.ttf", kBodySize);
        _italic  = load("IBMPlexSans-Italic.ttf",  kBodySize);
        _bold    = load("IBMPlexSans-Bold.ttf",    kBodySize);
        _boldH1  = load("IBMPlexSans-Bold.ttf",    30.0f * uiScale);
        _boldH2  = load("IBMPlexSans-Bold.ttf",    22.5f * uiScale);
        _boldH3  = load("IBMPlexSans-Bold.ttf",    18.0f * uiScale);
    }
}
