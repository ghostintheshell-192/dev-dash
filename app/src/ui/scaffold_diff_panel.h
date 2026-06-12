#pragma once

#include <set>
#include <string>
#include <vector>

#include "../core/diff_entry.h"
#include "../core/project.h"
#include "../core/scaffold.h"
#include "file_diff_panel.h"

namespace dev_dash::services
{
    class ScaffoldRepository;
    class DiffEngine;
    class PromoteEngine;
    class ApplyEngine;
    class SnapshotService;
}

namespace dev_dash::ui
{
    class DocumentPanelHost;
    class StatusSink;

    // Compares a scaffold against the project and lets the user apply
    // (scaffold → project) or promote (project → scaffold) the differences.
    // Scaffold management (create/delete/browse) lives in ScaffoldsView.
    class ScaffoldDiffPanel
    {
    public:
        ScaffoldDiffPanel(services::ScaffoldRepository& repo,
                          services::DiffEngine&         diffEngine,
                          services::PromoteEngine&      promoteEngine,
                          services::ApplyEngine&        applyEngine,
                          services::SnapshotService&    snapshotService,
                          DocumentPanelHost&            docHost,
                          StatusSink&                   status,
                          const core::Project&          project);

        void Render(bool* open);

        // Select the comparison scaffold by name (e.g. launched from the
        // sidebar). Unknown names keep the current selection.
        void SelectScaffold(const std::string& name);

        // The per-file diff viewer. Rendered by the shell every frame: its
        // lifetime must not depend on this panel's visibility, or docking
        // the viewer over the panel makes the two windows starve each other.
        FileDiffPanel& DiffViewer() { return _diffViewer; }

    private:
        void Refresh();
        void RunDiff();
        void RenderPromoteConfirmModal();
        void RenderApplyConfirmModal();

        services::ScaffoldRepository& _repo;
        services::DiffEngine&         _diffEngine;
        services::PromoteEngine&      _promoteEngine;
        services::ApplyEngine&        _applyEngine;
        services::SnapshotService&    _snapshotService;
        DocumentPanelHost&            _docHost;
        StatusSink&                   _status;
        core::Project                 _project;

        std::vector<core::Scaffold>   _scaffolds;
        int                           _selectedIdx = 0;
        std::vector<core::DiffEntry>  _diff;

        std::set<std::string>         _selectedForPromote;
        bool                          _showPromoteConfirm  = false;
        bool                          _showApplyConfirm    = false;
        FileDiffPanel                 _diffViewer;

        bool _needsRefresh = true;
    };
}
