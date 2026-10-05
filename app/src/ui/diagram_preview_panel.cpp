#include "diagram_preview_panel.h"

#include <cstring>

#include <imgui.h>

namespace dev_dash::ui
{
    namespace
    {
        // DevDash's own layers (ADR-010): a diagram with edge labels and a
        // few shapes, enough to judge the colours.
        constexpr const char* kSampleDot = R"(digraph layers {
    rankdir=TB
    node [shape=box]

    app      [label="app\ncomposition root"]
    ui       [label="ui\npanels"]
    services [label="services\ndomain logic"]
    platform [label="platform\nSDL3 + Vulkan"]
    core     [label="core\nvalue types", shape=ellipse]

    app -> ui
    app -> services
    app -> platform
    ui -> services [label="injected"]
    ui -> core
    services -> core
}
)";
    }

    DiagramPreviewPanel::DiagramPreviewPanel()
    {
        std::strncpy(_source.data(), kSampleDot, _source.size() - 1);
        ImGuiDot::Update(_diagram, _source.data());
    }

    DiagramPreviewPanel::~DiagramPreviewPanel()
    {
        ImGuiDot::CleanUp(_diagram);
    }

    void DiagramPreviewPanel::Render(bool* open)
    {
        if (!ImGui::Begin("DOT preview", open))
        {
            ImGui::End();
            return;
        }

        // ── DOT source ──────────────────────────────────────────────────
        ImGui::BeginChild("##dot_source", ImVec2(_editorWidth, 0.0f),
                          ImGuiChildFlags_ResizeX);
        ImGui::TextDisabled("DOT source");
        // A parse error keeps the last valid diagram on screen.
        if (ImGui::InputTextMultiline("##dot", _source.data(), _source.size(),
                                      ImVec2(-1.0f, -1.0f),
                                      ImGuiInputTextFlags_AllowTabInput))
            ImGuiDot::Update(_diagram, _source.data());
        _editorWidth = ImGui::GetWindowWidth();
        ImGui::EndChild();

        ImGui::SameLine();

        // ── Diagram ─────────────────────────────────────────────────────
        ImGui::BeginChild("##dot_diagram", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None);
        ImGui::SetNextItemWidth(160.0f);
        ImGui::SliderFloat("Zoom", &_zoom, 0.25f, 4.0f, "%.2fx",
                           ImGuiSliderFlags_Logarithmic);
        ImGui::Separator();

        ImGui::BeginChild("##dot_canvas", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None,
                          ImGuiWindowFlags_HorizontalScrollbar);
        ImGuiDot::Draw(_diagram, _zoom, ImVec2(0.5f, 0.5f));
        ImGui::EndChild();

        ImGui::EndChild();

        ImGui::End();
    }
}
