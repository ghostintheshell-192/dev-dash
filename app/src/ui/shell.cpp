#include "shell.h"

#include "code_graph_panel.h"
#include "diagram_preview_panel.h"
#include "document_panel_host.h"
#include "effective_config_panel.h"
#include "scaffold_diff_panel.h"
#include "sidebar.h"
#include "snapshot_history_panel.h"
#include "theme.h"

#include <algorithm>

namespace dev_dash::ui
{
    namespace
    {
        constexpr float kSidebarMinWidth = 160.0f;
        constexpr float kSidebarMaxWidth = 480.0f;
        constexpr float kSplitterWidth   = 5.0f;
    }

    Shell::Shell(services::ConfigResolver&     configResolver,
                 services::ScaffoldRepository& scaffoldRepository,
                 services::DiffEngine&         diffEngine,
                 services::PromoteEngine&      promoteEngine,
                 services::ApplyEngine&        applyEngine,
                 services::SnapshotService&    snapshotService,
                 services::CppClassExtractor&  classExtractor,
                 services::ClassDiagramGenerator& diagramGenerator,
                 DocumentPanelHost&            docHost,
                 const core::Project&          project,
                 bool                          diagramsAvailable)
        : _project(project)
        , _docHost(docHost)
    {
        _docHost.SetStatusSink(&_status);

        _configView = std::make_unique<EffectiveConfigPanel>(
            configResolver, docHost, project);

        _diffView = std::make_unique<ScaffoldDiffPanel>(
            scaffoldRepository, diffEngine, promoteEngine, applyEngine,
            snapshotService, docHost, _status, project);

        _historyView = std::make_unique<SnapshotHistoryPanel>(
            snapshotService, _status, project);

        if (diagramsAvailable)
        {
            _diagramView   = std::make_unique<DiagramPreviewPanel>();
            _codeGraphView = std::make_unique<CodeGraphPanel>(classExtractor, diagramGenerator, _status, project);
        }

        Sidebar::Callbacks callbacks;
        callbacks.openDocument =
            [this](const std::filesystem::path& path) { _docHost.OpenPanel(path); };
        callbacks.compareScaffold = [this](const std::string& scaffoldName)
        {
            _diffOpen = true;
            _diffView->SelectScaffold(scaffoldName);
            ImGui::SetWindowFocus("Compare");
        };
        callbacks.openConfigView = [this]
        {
            _configOpen = true;
            ImGui::SetWindowFocus("Config");
        };
        callbacks.openHistoryView = [this]
        {
            _historyOpen = true;
            ImGui::SetWindowFocus("History");
        };

        _sidebar = std::make_unique<Sidebar>(
            configResolver, scaffoldRepository, snapshotService,
            _status, project, std::move(callbacks));
    }

    // The document host outlives the shell (it is owned by App): detach the
    // status bar before it goes.
    Shell::~Shell()
    {
        _docHost.SetStatusSink(nullptr);
    }

    void Shell::Render()
    {
        const ImGuiIO& io = ImGui::GetIO();

        // ── Host window: fixed chrome, no padding, no docking onto it ──────
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::Begin("##shell", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                         | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
                         | ImGuiWindowFlags_NoBringToFrontOnFocus
                         | ImGuiWindowFlags_NoDocking);
        ImGui::PopStyleVar(2);

        RenderTopBar();

        const float statusH = ImGui::GetFrameHeightWithSpacing();

        // ── Sidebar ─────────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_ChildBg, CurrentTheme().panelBg);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::BeginChild("##sidebar",
                          ImVec2(_sidebarWidth, -statusH),
                          ImGuiChildFlags_AlwaysUseWindowPadding);
        _sidebar->Render();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImGui::SameLine(0.0f, 0.0f);
        RenderSidebarSplitter(statusH);
        ImGui::SameLine(0.0f, 0.0f);

        // ── Central dockspace: views and documents live here as tabs ───────
        _dockspaceId = ImGui::GetID("##workspace");
        ImGui::DockSpace(_dockspaceId, ImVec2(0, -statusH));

        RenderStatusBar();

        ImGui::End();

        // ── Views (dock into the workspace on first open) ───────────────────
        if (_configOpen)
        {
            ImGui::SetNextWindowDockID(_dockspaceId, ImGuiCond_FirstUseEver);
            _configView->Render(&_configOpen);
        }
        if (_diffOpen)
        {
            ImGui::SetNextWindowDockID(_dockspaceId, ImGuiCond_FirstUseEver);
            _diffView->Render(&_diffOpen);
        }
        if (_historyOpen)
        {
            ImGui::SetNextWindowDockID(_dockspaceId, ImGuiCond_FirstUseEver);
            _historyView->Render(&_historyOpen);
        }
        if (_diagramOpen && _diagramView)
        {
            ImGui::SetNextWindowDockID(_dockspaceId, ImGuiCond_FirstUseEver);
            _diagramView->Render(&_diagramOpen);
        }
        if (_codeGraphOpen && _codeGraphView)
        {
            ImGui::SetNextWindowDockID(_dockspaceId, ImGuiCond_FirstUseEver);
            _codeGraphView->Render(&_codeGraphOpen);
        }
        // The diagram tabs render every frame, apart from the class list
        // that opens them: closing the list must not close the diagrams.
        if (_codeGraphView)
            _codeGraphView->RenderDiagrams(_dockspaceId);

        // The file diff viewer renders every frame, independently of the
        // scaffold diff panel that opened it: if it only rendered while
        // that panel was visible, docking the viewer over the panel would
        // make the two windows starve each other (tab flicker).
        if (_diffView->DiffViewer().IsOpen())
        {
            ImGui::SetNextWindowDockID(_dockspaceId, ImGuiCond_FirstUseEver);
            _diffView->DiffViewer().Render();
        }

        _docHost.Render(_dockspaceId);
    }

    void Shell::RenderTopBar()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 6));
        ImGui::BeginChild("##topbar", ImVec2(0, ImGui::GetFrameHeight() + 12),
                          ImGuiChildFlags_AlwaysUseWindowPadding);

        ImGui::TextColored(CurrentTheme().accent, "%s",
                           _project.path.filename().string().c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("%s", _project.path.string().c_str());

        const ImGuiStyle& style = ImGui::GetStyle();
        const float switchW  = ImGui::CalcTextSize("Change project...").x
                               + style.FramePadding.x * 2.0f;
        const auto buttonW = [&](const char* label)
        { return ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f + style.ItemSpacing.x; };
        const float diagramW = _diagramView ? buttonW("Code graph") + buttonW("Diagram") : 0.0f;
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - switchW - diagramW
                        + ImGui::GetCursorPosX());
        if (_codeGraphView)
        {
            if (ImGui::SmallButton("Code graph"))
            {
                _codeGraphOpen = true;
                ImGui::SetWindowFocus("Code graph");
            }
            ImGui::SameLine();
        }
        if (_diagramView)
        {
            if (ImGui::SmallButton("Diagram"))
            {
                _diagramOpen = true;
                ImGui::SetWindowFocus("Diagram");
            }
            ImGui::SameLine();
        }
        if (ImGui::SmallButton("Change project..."))
            _wantsProjectSwitch = true;

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::Separator();
    }

    void Shell::RenderSidebarSplitter(float statusH)
    {
        ImGui::InvisibleButton("##sidebar_splitter",
                               ImVec2(kSplitterWidth, -statusH));
        if (ImGui::IsItemActive())
        {
            _sidebarWidth += ImGui::GetIO().MouseDelta.x;
            _sidebarWidth = std::clamp(_sidebarWidth,
                                       kSidebarMinWidth, kSidebarMaxWidth);
        }
        if (ImGui::IsItemHovered() || ImGui::IsItemActive())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        const Theme& t = CurrentTheme();
        const ImVec4 color = ImGui::IsItemActive()    ? t.accent
                           : ImGui::IsItemHovered()   ? t.accentHover
                                                      : t.border;
        ImGui::GetWindowDrawList()->AddRectFilled(
            ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
            ImGui::GetColorU32(color));
    }

    void Shell::RenderStatusBar()
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, CurrentTheme().panelBg);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 4));
        ImGui::BeginChild("##statusbar", ImVec2(0, 0),
                          ImGuiChildFlags_AlwaysUseWindowPadding);

        if (const std::string hint = _status.TakeHint(); !hint.empty())
        {
            ImGui::TextUnformatted(hint.c_str());
        }
        else if (!_status.Message().empty())
        {
            const Theme& t = CurrentTheme();
            const ImVec4 color =
                _status.GetLevel() == StatusSink::Level::kError   ? t.removed
              : _status.GetLevel() == StatusSink::Level::kSuccess ? t.added
                                                                  : t.text;
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(_status.Message().c_str());
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::TextDisabled("Ready");
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }
}
