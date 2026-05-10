#include "font_library.h"

#include <iostream>
#include <string>

#include <SDL3/SDL.h>
#include <imgui.h>

namespace dev_dash::ui
{
    FontLibrary::FontLibrary()
    {
        std::string fontsDir;
        if (const char* base = SDL_GetBasePath())
            fontsDir = base;
        fontsDir += "assets/fonts/";

        ImFontAtlas* atlas = ImGui::GetIO().Fonts;
        constexpr float kBodySize = 16.0f;

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
        constexpr const char* kDejaVu = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";

        auto load = [&](const char* file, float size) -> ImFont*
        {
            ImFont* f = atlas->AddFontFromFileTTF(
                (fontsDir + file).c_str(), size, nullptr, kPrimaryRanges);
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

        _regular = load("IBMPlexSans-Regular.ttf", kBodySize);
        _italic  = load("IBMPlexSans-Italic.ttf",  kBodySize);
        _bold    = load("IBMPlexSans-Bold.ttf",    kBodySize);
        _boldH1  = load("IBMPlexSans-Bold.ttf",    30.0f);
        _boldH2  = load("IBMPlexSans-Bold.ttf",    22.5f);
        _boldH3  = load("IBMPlexSans-Bold.ttf",    17.55f);
    }
}
