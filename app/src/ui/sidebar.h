#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "../core/effective_config.h"
#include "../core/project.h"
#include "../core/scaffold.h"
#include "../core/snapshot.h"

namespace dev_dash::services
{
    class ConfigResolver;
    class ScaffoldRepository;
    class SnapshotService;
}

namespace dev_dash::ui
{
    class StatusSink;

    // VS Code-style navigation sidebar: collapsible sections with trees and
    // lists. The rule it enforces: sidebar = structure (what exists, where),
    // workspace = content (what you are reading or acting on). A future
    // resource (ideas, specs, tech-debt...) is one more section here.
    class Sidebar
    {
    public:
        struct Callbacks
        {
            std::function<void(const std::filesystem::path&)> openDocument;
            std::function<void(const std::string&)>           compareScaffold;
            std::function<void()>                             openConfigView;
            std::function<void()>                             openHistoryView;
        };

        Sidebar(services::ConfigResolver&     configResolver,
                services::ScaffoldRepository& scaffoldRepository,
                services::SnapshotService&    snapshotService,
                StatusSink&                   status,
                const core::Project&          project,
                Callbacks                     callbacks);

        // Renders the sidebar content (the shell owns the surrounding child
        // window and splitter).
        void Render();

    private:
        // Nested directory tree built from flat relative paths.
        struct FileTree
        {
            std::map<std::string, FileTree> dirs;
            std::vector<std::string>        files;
        };

        void RenderConfigSection();
        void RenderScaffoldsSection();
        void RenderHistorySection();
        void RenderFileTree(const FileTree&              node,
                            const std::filesystem::path& root,
                            const std::filesystem::path& relBase);
        void RenderNewScaffoldModal();
        void RenderDeleteConfirmModal();
        void Refresh();

        static FileTree BuildFileTree(const std::vector<std::string>& paths);

        services::ConfigResolver&     _configResolver;
        services::ScaffoldRepository& _scaffoldRepo;
        services::SnapshotService&    _snapshotService;
        StatusSink&                   _status;
        core::Project                 _project;
        Callbacks                     _callbacks;

        core::EffectiveConfig           _config;
        std::vector<core::Scaffold>     _scaffolds;
        std::map<std::string, FileTree> _scaffoldFiles;  // by scaffold name, lazy
        std::vector<core::Snapshot>     _snapshots;
        std::string                     _projectSlug;

        bool        _showNewModal      = false;
        bool        _showDeleteConfirm = false;
        char        _newName[128]      = {};
        int         _newMode           = 0;  // 0=empty, 1=copy
        std::string _modalScaffoldName;      // context scaffold for new/delete
        bool        _needsRefresh = true;
    };
}
