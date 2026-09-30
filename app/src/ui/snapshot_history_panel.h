#pragma once

#include <string>
#include <vector>

#include "../core/project.h"
#include "../core/snapshot.h"

namespace dev_dash::services
{
    class SnapshotService;
}

namespace dev_dash::ui
{
    class StatusSink;

    class SnapshotHistoryPanel
    {
    public:
        SnapshotHistoryPanel(services::SnapshotService& service,
                             StatusSink&                 status,
                             const core::Project&        project);

        void Render(bool* open);

    private:
        void Refresh();
        void RenderSaveModal();
        void RenderRestoreConfirmModal();

        services::SnapshotService& _service;
        StatusSink&                _status;
        core::Project              _project;
        std::string                _projectSlug;
        std::vector<core::Snapshot> _snapshots;
        int                        _selectedIdx      = -1;
        bool                       _showSaveModal    = false;
        bool                       _showRestoreConfirm = false;
        char                       _newName[128]     = {};
        char                       _newDesc[256]     = {};
        bool                       _needsRefresh     = true;
    };
}
