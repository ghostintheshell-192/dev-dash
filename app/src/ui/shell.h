#pragma once

#include <memory>

#include <imgui.h>

#include "../core/project.h"
#include "status_sink.h"

namespace dev_dash::services
{
    class ConfigResolver;
    class ScaffoldRepository;
    class DiffEngine;
    class PromoteEngine;
    class ApplyEngine;
    class SnapshotService;
}

namespace dev_dash::ui
{
    class DocumentPanelHost;
    class EffectiveConfigPanel;
    class Sidebar;
    class ScaffoldDiffPanel;
    class SnapshotHistoryPanel;
    class DiagramPreviewPanel;

    // The workspace shell: top bar, navigation sidebar, central dockspace,
    // status bar. The sidebar holds structure (trees, lists — see Sidebar);
    // the workspace holds content (documents, diffs, tables) as dockable
    // tabs. There is no back-stack.
    class Shell
    {
    public:
        Shell(services::ConfigResolver&     configResolver,
              services::ScaffoldRepository& scaffoldRepository,
              services::DiffEngine&         diffEngine,
              services::PromoteEngine&      promoteEngine,
              services::ApplyEngine&        applyEngine,
              services::SnapshotService&    snapshotService,
              DocumentPanelHost&            docHost,
              const core::Project&          project,
              bool                          diagramsAvailable);
        ~Shell();

        void Render();

        // True when the user asked to go back to project selection.
        bool WantsProjectSwitch() const { return _wantsProjectSwitch; }

    private:
        void RenderTopBar();
        void RenderSidebarSplitter(float availableHeight);
        void RenderStatusBar();

        core::Project      _project;
        DocumentPanelHost& _docHost;
        StatusSink         _status;

        std::unique_ptr<Sidebar>              _sidebar;
        std::unique_ptr<EffectiveConfigPanel> _configView;
        std::unique_ptr<ScaffoldDiffPanel>    _diffView;
        std::unique_ptr<SnapshotHistoryPanel> _historyView;
        std::unique_ptr<DiagramPreviewPanel>  _diagramView;   // null when ImGuiDot is unavailable

        bool _configOpen  = true;
        bool _diffOpen    = false;
        bool _historyOpen = false;
        bool _diagramOpen = false;

        float   _sidebarWidth       = 240.0f;
        ImGuiID _dockspaceId        = 0;
        bool    _wantsProjectSwitch = false;
    };
}
