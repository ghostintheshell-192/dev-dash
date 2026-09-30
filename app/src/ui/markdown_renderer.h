#pragma once

#include <functional>
#include <string_view>

#include <imgui_md.h>

namespace dev_dash::ui
{
    class FontLibrary;

    class MarkdownRenderer : public imgui_md
    {
    public:
        using LinkHandler = std::function<void(std::string_view url)>;

        explicit MarkdownRenderer(const FontLibrary& fonts);
        ~MarkdownRenderer() = default;

        MarkdownRenderer(const MarkdownRenderer&)            = delete;
        MarkdownRenderer& operator=(const MarkdownRenderer&) = delete;

        // Default handler opens external URLs via OpenExternalUrl().
        void SetLinkHandler(LinkHandler handler);

        // Hands `url` to the system (browser, file manager, default app).
        // Returns false, and logs why, if the system refuses it.
        static bool OpenExternalUrl(std::string_view url);

    protected:
        void get_font(font_info& info) const override;
        void open_url() const override;
        bool get_image(image_info&) const override { return false; }
        void SPAN_CODE(bool e) override;
        void BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool e) override;

    private:
        const FontLibrary& _fonts;
        LinkHandler _linkHandler;
    };
}
