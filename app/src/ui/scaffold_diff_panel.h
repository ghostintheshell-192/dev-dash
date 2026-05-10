#pragma once

#include <vector>

#include "../core/diff_entry.h"
#include "../core/project.h"
#include "../core/scaffold.h"

namespace dev_dash::services
{
    class ScaffoldRepository;
    class DiffEngine;
}

namespace dev_dash::ui { class DocumentPanelHost; }

namespace dev_dash::ui
{
    class ScaffoldDiffPanel
    {
    public:
        ScaffoldDiffPanel(services::ScaffoldRepository& repo,
                          services::DiffEngine&         diffEngine,
                          DocumentPanelHost&            docHost,
                          const core::Project&          project);

        void Render();
        bool WantsBack() const { return _wantsBack; }

    private:
        void Refresh();
        void RunDiff();

        services::ScaffoldRepository& _repo;
        services::DiffEngine&         _diffEngine;
        DocumentPanelHost&            _docHost;
        core::Project                 _project;

        std::vector<core::Scaffold>   _scaffolds;
        int                           _selectedIdx = 0;
        std::vector<core::DiffEntry>  _diff;

        bool _wantsBack     = false;
        bool _needsRefresh  = true;
    };
}
