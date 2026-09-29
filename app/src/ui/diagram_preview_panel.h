#pragma once

#include <array>

#include <ImGuiDot.h>

namespace dev_dash::ui
{
    // A DOT editor next to the diagram ImGuiDot draws from it, redrawn as the
    // source changes. A first home for ImGuiDot in the app: it shows how the
    // diagrams look with the current theme, ahead of the code graph
    // (feature-code-graph). ImGuiDot::Initialize() must have succeeded.
    class DiagramPreviewPanel
    {
    public:
        DiagramPreviewPanel();
        ~DiagramPreviewPanel();

        DiagramPreviewPanel(const DiagramPreviewPanel&)            = delete;
        DiagramPreviewPanel& operator=(const DiagramPreviewPanel&) = delete;

        void Render(bool* open);

    private:
        ImGuiDot::DiagramState  _diagram;
        std::array<char, 16384> _source{};
        float                   _zoom        = 1.0f;
        float                   _editorWidth = 320.0f;
    };
}
