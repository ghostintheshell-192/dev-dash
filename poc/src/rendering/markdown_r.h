#pragma once

#include "imgui_md.h"

namespace Rendering
{
    struct MarkdownFonts
    {
        // IBM Plex Sans family loaded at init time; null until InitImGui() runs.
        ImFont *regular = nullptr;
        ImFont *italic = nullptr;
        ImFont *bold = nullptr;
        ImFont *boldH1 = nullptr;
        ImFont *boldH2 = nullptr;
        ImFont *boldH3 = nullptr;
    };

    class MarkdownRenderer : public imgui_md
    {
    public:
        MarkdownRenderer(std::vector<std::string>& pendingPanels);
        void InitFonts();

    protected:
        ImFont *get_font() const override;

        void open_url() const override;

        bool get_image(image_info &nfo) const override;
        void SPAN_CODE(bool e) override; // per il colore code inline
        void BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool e)  override;

    private:
        MarkdownFonts _fonts;
        std::vector<std::string> &_pendingPanels; // queue owned by Renderer; we just push
    };

}
