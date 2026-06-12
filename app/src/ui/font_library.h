#pragma once

#include <imgui.h>

namespace dev_dash::ui
{
    class FontLibrary
    {
    public:
        // Must be called after ImGui::CreateContext() and before ImGui_ImplVulkan_Init().
        // uiScale multiplies every font size (display content scale, ≥ 1).
        explicit FontLibrary(float uiScale = 1.0f);
        ~FontLibrary() = default;

        FontLibrary(const FontLibrary&)            = delete;
        FontLibrary& operator=(const FontLibrary&) = delete;

        ImFont* Regular() const { return _regular; }
        ImFont* Italic()  const { return _italic; }
        ImFont* Bold()    const { return _bold; }
        ImFont* BoldH1()  const { return _boldH1; }
        ImFont* BoldH2()  const { return _boldH2; }
        ImFont* BoldH3()  const { return _boldH3; }

    private:
        ImFont* _regular = nullptr;
        ImFont* _italic  = nullptr;
        ImFont* _bold    = nullptr;
        ImFont* _boldH1  = nullptr;
        ImFont* _boldH2  = nullptr;
        ImFont* _boldH3  = nullptr;
    };
}
