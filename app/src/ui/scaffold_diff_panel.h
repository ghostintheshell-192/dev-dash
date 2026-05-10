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
                          DocumentPanelHost&            docHost,
                          const core::Project&          project);

        void Render();
        bool WantsBack() const { return _wantsBack; }

    private:
        void Refresh();
        void RunDiff();
        void RenderPromoteConfirmModal();
        void RenderNewScaffoldModal();
        void RenderDeleteConfirmModal();

        services::ScaffoldRepository& _repo;
        services::DiffEngine&         _diffEngine;
        services::PromoteEngine&      _promoteEngine;
        DocumentPanelHost&            _docHost;
        core::Project                 _project;

        std::vector<core::Scaffold>   _scaffolds;
        int                           _selectedIdx = 0;
        std::vector<core::DiffEntry>  _diff;

        std::set<std::string>         _selectedForPromote;
        bool                          _showPromoteConfirm  = false;
        bool                          _showNewModal        = false;
        bool                          _showDeleteConfirm   = false;
        char                          _newName[128]        = {};
        int                           _newMode             = 0;  // 0=empty, 1=copy
        std::string                   _statusMsg;
        FileDiffPanel                 _diffViewer;

        bool _wantsBack    = false;
        bool _needsRefresh = true;
    };
}
