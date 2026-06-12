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

namespace dev_dash::ui { class DocumentPanelHost; }

namespace dev_dash::ui
{
    class ScaffoldDiffPanel
    {
    public:
        ScaffoldDiffPanel(services::ScaffoldRepository& repo,
                          services::DiffEngine&         diffEngine,
                          services::PromoteEngine&      promoteEngine,
                          services::ApplyEngine&        applyEngine,
                          services::SnapshotService&    snapshotService,
                          DocumentPanelHost&            docHost,
                          const core::Project&          project);

        void Render();
        bool WantsBack() const { return _wantsBack; }

    private:
        void Refresh();
        void RunDiff();
        void RenderPromoteConfirmModal();
        void RenderApplyConfirmModal();
        void RenderNewScaffoldModal();
        void RenderDeleteConfirmModal();

        services::ScaffoldRepository& _repo;
        services::DiffEngine&         _diffEngine;
        services::PromoteEngine&      _promoteEngine;
        services::ApplyEngine&        _applyEngine;
        services::SnapshotService&    _snapshotService;
        DocumentPanelHost&            _docHost;
        core::Project                 _project;

        std::vector<core::Scaffold>   _scaffolds;
        int                           _selectedIdx = 0;
        std::vector<core::DiffEntry>  _diff;

        enum class StatusLevel { kInfo, kSuccess, kError };

        std::set<std::string>         _selectedForPromote;
        bool                          _showPromoteConfirm  = false;
        bool                          _showApplyConfirm    = false;
        bool                          _showNewModal        = false;
        bool                          _showDeleteConfirm   = false;
        char                          _newName[128]        = {};
        int                           _newMode             = 0;  // 0=empty, 1=copy
        std::string                   _statusMsg;
        StatusLevel                   _statusLevel = StatusLevel::kInfo;
        FileDiffPanel                 _diffViewer;

        bool _wantsBack    = false;
        bool _needsRefresh = true;
    };
}
