#pragma once

#include <array>
#include <filesystem>
#include <future>
#include <map>
#include <memory>
#include <set>
#include <stop_token>
#include <string>
#include <vector>

#include <imgui.h>

#include "../core/code_model.h"
#include "../core/project.h"
#include "../services/cpp_class_extractor.h"

namespace dev_dash::services { class ClassDiagramGenerator; }

namespace dev_dash::ui
{
    class StatusSink;

    // The code graph (feature-code-graph): reads the C++ classes of the
    // project in the background (US-0); what was read, and from which
    // folders, is in the Code analysis tab. "New diagram" opens a diagram in
    // a tab of its own (US-2), with its own selection: the tab holds the
    // filters, a tree of the classes with a check box at every level (US-1),
    // and every change draws the checked classes again, with their direct
    // neighbours. Several diagrams stay open side by side; closing a tab is
    // how a diagram goes away. ImGuiDot::Initialize() must have succeeded.
    class CodeGraphPanel
    {
    public:
        CodeGraphPanel(services::CppClassExtractor& extractor,
                       services::ClassDiagramGenerator& generator,
                       StatusSink& status,
                       const core::Project& project);
        ~CodeGraphPanel();

        CodeGraphPanel(const CodeGraphPanel&)            = delete;
        CodeGraphPanel& operator=(const CodeGraphPanel&) = delete;

        // The sidebar section: the commands Analyze code and New diagram.
        void RenderSidebarSection();
        // The Code analysis tab, for the View menu.
        bool IsAnalysisOpen() const { return _analysisOpen; }
        void ShowAnalysis(bool show);
        // The Code analysis tab and the diagram tabs, docked into the
        // workspace on first show. Called every frame, also with the sidebar
        // section collapsed: it collects the reading when it is done.
        void RenderTabs(ImGuiID dockspaceId);

    private:
        // A scope (namespace or enclosing class) in the tree of the classes:
        // the scopes nested in it, by name, and its own classes, sorted.
        struct ScopeNode
        {
            std::map<std::string, ScopeNode> children;
            std::vector<std::size_t>         classes;   // indices into the model
        };

        // A directory of the project holding C++ files, in the tree of the
        // folders to read.
        struct DirectoryNode
        {
            std::filesystem::path                relative;   // to the project root
            std::map<std::string, DirectoryNode> children;
        };

        // How the tree lists the classes.
        enum class ClassView
        {
            kNamespaces,   // by namespace (and enclosing class)
            kFolders,      // by the folder of the file declaring them
        };

        struct DiagramTab;

        void StartScan();
        void CollectScan();
        void SetSourceDirectories(const std::vector<std::filesystem::path>& directories);
        void StartReading();
        void CollectReading();
        bool FoldersChanged() const;
        void RenderAnalysisTab();
        void RenderFolders();
        void RenderFilesRead();
        void RenderPartlyRead();
        void RenderDirectory(const DirectoryNode& node, bool parentExcluded);
        bool IsExcluded(const std::filesystem::path& relative) const;
        bool HasExcludedBelow(const std::filesystem::path& relative) const;
        void SetExcluded(const std::filesystem::path& relative, bool excluded);
        void RenderFilters(DiagramTab& tab);
        // Return whether a box changed the selection of the tab.
        bool RenderScope(DiagramTab& tab, const ScopeNode& node, const std::string& path, int depth);
        bool RenderClass(DiagramTab& tab, std::size_t index);
        void CollectVisible(const DiagramTab& tab, const ScopeNode& node, std::vector<std::size_t>& visible) const;
        void CreateDiagram();
        void GenerateDiagram(DiagramTab& tab);
        void RenderDiagramTab(DiagramTab& tab);
        void RenderToolbar(DiagramTab& tab);
        void RenderCanvas(DiagramTab& tab, float width);
        void HandleViewInput(DiagramTab& tab, const ImVec2& origin);

        services::CppClassExtractor&     _extractor;
        services::ClassDiagramGenerator& _generator;
        StatusSink&                      _status;
        core::Project                    _project;

        // The folders are found when the analysis tab first opens, before
        // any reading: a walk of the tree, on a worker thread.
        std::future<services::SourceScan>  _scanning;
        std::vector<std::filesystem::path> _sourceDirectories;   // relative to the project root
        bool                               _scanned = false;
        // Close the folders on the next frame: a reading is done, the room
        // goes to what it found.
        bool                               _closeFolders = false;

        // Reading runs on a worker thread; the result is collected on the UI
        // thread once ready.
        std::future<services::ExtractionResult> _reading;
        std::stop_source                        _stopReading;
        services::ExtractionResult              _result;
        bool                                    _hasResult = false;
        std::string                             _readError;
        std::string                             _readAt;      // "14:32", when the result was collected
        ScopeNode                               _tree;        // the global scope
        ScopeNode                               _folderTree;  // the project root, folders as scopes
        DirectoryNode                           _directories;   // the project root

        // The folders left out of the reading, relative to the project root.
        // Empty until the folders are found, with the default rule of the
        // extractor (test directories out).
        std::set<std::filesystem::path> _excluded;
        bool                            _excludedChosen = false;

        // The options last chosen in a diagram: those of the next one.
        bool                         _showNeighbours = true;
        std::set<core::MemberAccess> _shownAccess    = {core::MemberAccess::kPublic};

        // The Code analysis tab: shown, and brought to the front on the
        // next frame.
        bool _analysisOpen  = false;
        bool _analysisFocus = false;

        std::vector<std::unique_ptr<DiagramTab>> _tabs;
        int                                      _nextTabNumber = 1;
    };
}
