#pragma once

#include <array>
#include <future>
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
    // project in the background and lists them by scope with a check box
    // each. "Create diagram" opens the class diagram of the checked classes,
    // with their direct neighbours, in a tab of its own: several diagrams can
    // stay open side by side, and closing a tab is how a diagram goes away.
    // The tabs live apart from the list: closing the list leaves them open.
    // The list moves to the sidebar later (US-1). ImGuiDot::Initialize()
    // must have succeeded.
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

        // The class list window.
        void Render(bool* open);
        // The diagram tabs, docked into the workspace on first show. Called
        // every frame, whether the list window is open or not.
        void RenderDiagrams(ImGuiID dockspaceId);

    private:
        // The classes of one scope (namespace or enclosing class), sorted.
        struct Group
        {
            std::string scope;
            std::vector<std::size_t> classes;   // indices into the model
        };

        struct DiagramTab;

        void StartReading();
        void CollectReading();
        void RenderClassList();
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
        std::vector<Group>                      _groups;

        std::set<std::string>  _selection;
        std::array<char, 128>  _filter{};
        bool                   _showNeighbours = true;
        bool                   _allMembers     = false;

        std::vector<std::unique_ptr<DiagramTab>> _tabs;
        int                                      _nextTabNumber = 1;
    };
}
