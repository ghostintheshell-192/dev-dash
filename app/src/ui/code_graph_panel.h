#pragma once

#include <array>
#include <future>
#include <set>
#include <stop_token>
#include <string>
#include <vector>

#include <ImGuiDot.h>

#include "../core/project.h"
#include "../services/cpp_class_extractor.h"

namespace dev_dash::services { class ClassDiagramGenerator; }

namespace dev_dash::ui
{
    // The code graph (feature-code-graph): reads the C++ classes of the
    // project in the background, lists them by scope with a check box each,
    // and draws the class diagram of the checked ones with their direct
    // neighbours. A first, self-contained home: the class list moves to the
    // sidebar later (US-1). ImGuiDot::Initialize() must have succeeded.
    class CodeGraphPanel
    {
    public:
        CodeGraphPanel(services::CppClassExtractor& extractor,
                       services::ClassDiagramGenerator& generator,
                       const core::Project& project);
        ~CodeGraphPanel();

        CodeGraphPanel(const CodeGraphPanel&)            = delete;
        CodeGraphPanel& operator=(const CodeGraphPanel&) = delete;

        void Render(bool* open);

    private:
        // The classes of one scope (namespace or enclosing class), sorted.
        struct Group
        {
            std::string scope;
            std::vector<std::size_t> classes;   // indices into the model
        };

        void StartReading();
        void CollectReading();
        void RenderClassList();
        void RenderDiagram();
        void CreateDiagram();

        services::CppClassExtractor&     _extractor;
        services::ClassDiagramGenerator& _generator;
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

        ImGuiDot::DiagramState _diagram;
        std::string            _dot;
        float                  _zoom       = 1.0f;
        bool                   _fitPending = false;  // fit the zoom to the view on the next frame
        float                  _listWidth  = 320.0f;
    };
}
