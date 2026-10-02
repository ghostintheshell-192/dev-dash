#pragma once

#include <array>
#include <future>
#include <map>
#include <memory>
#include <set>
#include <stop_token>
#include <string>
#include <vector>

#include <imgui.h>

#include "../core/project.h"
#include "../services/cpp_class_extractor.h"

namespace dev_dash::services { class ClassDiagramGenerator; }

namespace dev_dash::ui
{
    class StatusSink;

    // The code graph (feature-code-graph): reads the C++ classes of the
    // project in the background and lists them in a section of the sidebar,
    // as a tree of namespaces with a check box at every level (US-1).
    // "Create diagram" opens the class diagram of the checked classes, with
    // their direct neighbours, in a tab of its own in the workspace (US-2):
    // several diagrams can stay open side by side, and closing a tab is how
    // a diagram goes away. ImGuiDot::Initialize() must have succeeded.
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

        // The content of the sidebar section: reading, diagram options and
        // the tree of the classes.
        void RenderSidebarSection();
        // The diagram tabs, docked into the workspace on first show. Called
        // every frame, also with the sidebar section collapsed: it collects
        // the reading when it is done.
        void RenderDiagrams(ImGuiID dockspaceId);

    private:
        // A scope (namespace or enclosing class) in the tree of the classes:
        // the scopes nested in it, by name, and its own classes, sorted.
        struct ScopeNode
        {
            std::map<std::string, ScopeNode> children;
            std::vector<std::size_t>         classes;   // indices into the model
        };

        struct DiagramTab;

        void StartReading();
        void CollectReading();
        void RenderScope(const ScopeNode& node, const std::string& path, int depth);
        void RenderClass(std::size_t index);
        void CollectVisible(const ScopeNode& node, std::vector<std::size_t>& visible) const;
        void CreateDiagram();
        void RenderDiagramTab(DiagramTab& tab);

        services::CppClassExtractor&     _extractor;
        services::ClassDiagramGenerator& _generator;
        StatusSink&                      _status;
        core::Project                    _project;

        // Reading runs on a worker thread; the result is collected on the UI
        // thread once ready.
        std::future<services::ExtractionResult> _reading;
        std::stop_source                        _stopReading;
        services::ExtractionResult              _result;
        bool                                    _hasResult = false;
        std::string                             _readError;
        ScopeNode                               _tree;      // the global scope

        std::set<std::string>  _selection;
        std::array<char, 128>  _filter{};
        bool                   _showNeighbours = true;
        bool                   _allMembers     = false;

        std::vector<std::unique_ptr<DiagramTab>> _tabs;
        int                                      _nextTabNumber = 1;
    };
}
