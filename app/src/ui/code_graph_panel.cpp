#include "code_graph_panel.h"

#include "status_sink.h"
#include "theme.h"
#include "widgets.h"
#include "../services/class_diagram_generator.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <exception>
#include <map>

#include <ImGuiDot.h>
#include <imgui.h>

namespace dev_dash::ui
{
    namespace
    {
        constexpr std::string_view kScope = "::";

        constexpr float kMinZoom = 0.1f;
        constexpr float kMaxZoom = 4.0f;
        // Fitting never enlarges a small diagram past this.
        constexpr float kMaxFitZoom = 1.5f;

        std::string ScopeOf(const std::string& qualifiedName)
        {
            const std::size_t pos = qualifiedName.rfind(kScope);
            return pos == std::string::npos ? std::string() : qualifiedName.substr(0, pos);
        }

        std::string ShortNameOf(const std::string& qualifiedName)
        {
            const std::size_t pos = qualifiedName.rfind(kScope);
            return pos == std::string::npos ? qualifiedName : qualifiedName.substr(pos + kScope.size());
        }

        // Case-insensitive substring match, for the filter box.
        bool Matches(std::string_view text, std::string_view filter)
        {
            if (filter.empty())
                return true;
            const auto lower = [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); };
            return std::search(text.begin(), text.end(), filter.begin(), filter.end(),
                               [&](char a, char b) { return lower(a) == lower(b); })
                != text.end();
        }

        // "#rrggbb", or "#rrggbbaa" when not opaque: a Graphviz colour.
        std::string ToHex(const ImVec4& colour)
        {
            const auto byte = [](float v) { return static_cast<unsigned>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f); };
            char text[10];
            if (colour.w >= 1.0f)
                std::snprintf(text, sizeof(text), "#%02x%02x%02x", byte(colour.x), byte(colour.y), byte(colour.z));
            else
                std::snprintf(text, sizeof(text), "#%02x%02x%02x%02x", byte(colour.x), byte(colour.y), byte(colour.z),
                              byte(colour.w));
            return text;
        }
    }

    // One diagram, in a tab of its own: the DOT text it was made from (kept
    // for Copy DOT) and the state ImGuiDot laid out from it.
    struct CodeGraphPanel::DiagramTab
    {
        int                    number = 0;
        std::string            title;           // shown on the tab
        std::string            dot;
        ImGuiDot::DiagramState state;
        float                  zoom         = 1.0f;
        bool                   fitPending   = true;   // fit the zoom to the view on the next frame
        bool                   focusPending = true;   // bring the new tab to the front
        bool                   open         = true;

        DiagramTab() = default;
        DiagramTab(const DiagramTab&)            = delete;
        DiagramTab& operator=(const DiagramTab&) = delete;
        ~DiagramTab() { ImGuiDot::CleanUp(state); }
    };

    CodeGraphPanel::CodeGraphPanel(services::CppClassExtractor& extractor,
                                   services::ClassDiagramGenerator& generator,
                                   StatusSink& status,
                                   const core::Project& project)
        : _extractor(extractor)
        , _generator(generator)
        , _status(status)
        , _project(project)
    {
    }

    // A reading still running is asked to stop, then waited for by the
    // future's destructor: the worker stops at the next file.
    CodeGraphPanel::~CodeGraphPanel()
    {
        _stopReading.request_stop();
    }

    void CodeGraphPanel::StartReading()
    {
        _readError.clear();
        _stopReading = std::stop_source();
        _reading     = std::async(std::launch::async,
                                  [&extractor = _extractor, root = _project.path, stop = _stopReading.get_token()]
                                  { return extractor.Extract(root, stop); });
    }

    void CodeGraphPanel::CollectReading()
    {
        if (!_reading.valid() || _reading.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
            return;

        try
        {
            _result    = _reading.get();
            _hasResult = true;
        }
        catch (const std::exception& e)
        {
            _readError = e.what();
            _status.Set(StatusSink::Level::kError, "Code graph: reading failed: " + _readError);
            return;
        }
        _status.Set(StatusSink::Level::kInfo,
                    "Code graph: " + std::to_string(_result.model.classes.size()) + " classes read from "
                        + std::to_string(_result.filesRead) + " files");

        // Group by scope; drop the checked classes that no longer exist.
        std::map<std::string, std::vector<std::size_t>> byScope;
        std::set<std::string> names;
        for (std::size_t i = 0; i < _result.model.classes.size(); ++i)
        {
            const std::string& name = _result.model.classes[i].qualifiedName;
            byScope[ScopeOf(name)].push_back(i);
            names.insert(name);
        }
        _groups.clear();
        for (auto& [scope, classes] : byScope)
        {
            std::sort(classes.begin(), classes.end(), [&](std::size_t a, std::size_t b)
                      { return _result.model.classes[a].qualifiedName < _result.model.classes[b].qualifiedName; });
            _groups.push_back(Group{scope, std::move(classes)});
        }
        std::erase_if(_selection, [&](const std::string& name) { return !names.contains(name); });
    }

    void CodeGraphPanel::CreateDiagram()
    {
        const Theme& t = CurrentTheme();

        services::DiagramOptions options;
        options.showNeighbours = _showNeighbours;
        options.memberAccess   = _allMembers ? core::MemberAccess::kPrivate : core::MemberAccess::kPublic;
        // ImGuiDot does not draw records yet: plain boxes, one line per member.
        options.recordShapes        = false;
        options.palette.classFill   = ToHex(t.panelBg);
        options.palette.classBorder = ToHex(t.accent);
        options.palette.text        = ToHex(t.text);
        options.palette.ghostBorder = ToHex(t.textDim);
        options.palette.ghostText   = ToHex(t.textDim);
        options.palette.edge        = ToHex(t.textDim);

        auto tab    = std::make_unique<DiagramTab>();
        tab->number = _nextTabNumber++;
        tab->dot    = _generator.Generate(_result.model, _selection, options);
        ImGuiDot::Update(tab->state, tab->dot);

        // "Diagram 3: Shell, Sidebar +4": the first checked names say what
        // the diagram is about.
        constexpr std::size_t kNamesInTitle = 2;
        std::string names;
        std::size_t count = 0;
        for (const std::string& name : _selection)
        {
            if (count++ < kNamesInTitle)
                names += (names.empty() ? "" : ", ") + ShortNameOf(name);
        }
        if (count > kNamesInTitle)
            names += " +" + std::to_string(count - kNamesInTitle);
        tab->title = "Diagram " + std::to_string(tab->number) + ": " + names;

        _status.Set(StatusSink::Level::kInfo, "Code graph: " + tab->title + " opened");
        _tabs.push_back(std::move(tab));
    }

    void CodeGraphPanel::Render(bool* open)
    {
        CollectReading();

        if (!ImGui::Begin("Code graph", open))
        {
            ImGui::End();
            return;
        }

        RenderClassList();
        ImGui::End();
    }

    void CodeGraphPanel::RenderClassList()
    {
        const Theme& t       = CurrentTheme();
        const bool   reading = _reading.valid();

        // ----- Reading

        // The activity goes to the status bar; an item under the mouse
        // overrides it there while hovered.
        if (reading)
            _status.SetHint("Code graph: reading the C++ classes of the project in background...");

        ImGui::BeginDisabled(reading);
        if (ImGui::Button(reading ? "Reading..." : _hasResult ? "Read again" : "Read the code"))
            StartReading();
        ImGui::EndDisabled();
        StatusHint(_status, "Read the C++ classes of the project, skipping hidden and build directories "
                            "(.git, build, cmake-build-*, out, node_modules)");
        if (_hasResult && !reading)
        {
            ImGui::SameLine();
            ImGui::TextDisabled("%zu classes, %d files", _result.model.classes.size(), _result.filesRead);
        }

        if (!_readError.empty())
            ImGui::TextColored(t.removed, "%s", _readError.c_str());
        if (_hasResult && !_result.partlyReadFiles.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, t.modified);
            const bool expanded =
                ImGui::TreeNode("##partly_read", "%zu files read in part", _result.partlyReadFiles.size());
            ImGui::PopStyleColor();
            StatusHint(_status, "Code the reader did not understand: macros it cannot expand, syntax its "
                                "grammar misses, or real errors. The rest of each file is read.");
            if (expanded)
            {
                for (const std::filesystem::path& file : _result.partlyReadFiles)
                    ImGui::TextDisabled("%s", file.lexically_relative(_project.path).string().c_str());
                ImGui::TreePop();
            }
        }

        if (!_hasResult)
        {
            if (!reading)
                ImGui::TextWrapped("Reads the C++ classes of the project, skipping hidden and build directories.");
            return;
        }
        if (_result.model.classes.empty())
        {
            ImGui::TextWrapped(_result.filesRead == 0 ? "No C++ files in the project."
                                                      : "No classes in the C++ files of the project.");
            return;
        }

        // ----- Diagram options

        ImGui::Separator();
        ImGui::BeginDisabled(_selection.empty());
        if (ImGui::Button("Create diagram"))
            CreateDiagram();
        StatusHint(_status, _selection.empty() ? "Check at least one class to create a diagram"
                                               : "Open the diagram of the checked classes in a new tab, "
                                                 "with the options below");
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
            _selection.clear();
        StatusHint(_status, "Uncheck all the classes, also those hidden by the filter");
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextDisabled("%zu checked", _selection.size());
        ImGui::Checkbox("Neighbours", &_showNeighbours);
        StatusHint(_status, "Also draw the classes one step away from the checked ones, dimmed and "
                            "with their name only");
        ImGui::SameLine();
        ImGui::Checkbox("All members", &_allMembers);
        StatusHint(_status, "Show the protected and private members too, not only the public ones");

        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint("##filter", "Filter classes", _filter.data(), _filter.size());
        StatusHint(_status, "Show only the classes whose full name contains this text (case ignored)");
        const std::string_view filter(_filter.data());

        // ----- Classes by scope

        ImGui::BeginChild("##class_tree");
        for (const Group& group : _groups)
        {
            std::vector<std::size_t> visible;
            for (const std::size_t index : group.classes)
                if (Matches(_result.model.classes[index].qualifiedName, filter))
                    visible.push_back(index);
            if (visible.empty())
                continue;

            ImGui::PushID(group.scope.c_str());

            // One box for the whole scope: checked when all its visible
            // classes are; a click checks or clears them all.
            bool all = std::all_of(visible.begin(), visible.end(), [&](std::size_t index)
                                   { return _selection.contains(_result.model.classes[index].qualifiedName); });
            const bool changed = ImGui::Checkbox("##all", &all);
            StatusHint(_status, "Check or uncheck every class of "
                                    + (group.scope.empty() ? std::string("the global scope") : group.scope)
                                    + " shown by the filter");
            if (changed)
                for (const std::size_t index : visible)
                {
                    const std::string& name = _result.model.classes[index].qualifiedName;
                    if (all)
                        _selection.insert(name);
                    else
                        _selection.erase(name);
                }
            ImGui::SameLine();

            const std::string label = group.scope.empty() ? "(global scope)" : group.scope;
            if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
            {
                for (const std::size_t index : visible)
                {
                    const core::CodeClass& cls = _result.model.classes[index];
                    bool checked = _selection.contains(cls.qualifiedName);
                    ImGui::PushID(static_cast<int>(index));
                    if (ImGui::Checkbox(ShortNameOf(cls.qualifiedName).c_str(), &checked))
                    {
                        if (checked)
                            _selection.insert(cls.qualifiedName);
                        else
                            _selection.erase(cls.qualifiedName);
                    }
                    StatusHint(_status, cls.qualifiedName + "  ·  "
                                            + cls.file.lexically_relative(_project.path).string() + "  ·  "
                                            + std::to_string(cls.members.size()) + " members");
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    void CodeGraphPanel::RenderDiagrams(ImGuiID dockspaceId)
    {
        for (const std::unique_ptr<DiagramTab>& tab : _tabs)
        {
            ImGui::SetNextWindowDockID(dockspaceId, ImGuiCond_FirstUseEver);
            if (tab->focusPending)
            {
                ImGui::SetNextWindowFocus();
                tab->focusPending = false;
            }
            RenderDiagramTab(*tab);
        }
        std::erase_if(_tabs, [](const std::unique_ptr<DiagramTab>& tab) { return !tab->open; });
    }

    void CodeGraphPanel::RenderDiagramTab(DiagramTab& tab)
    {
        // The number after ### keeps the window identity when titles repeat.
        const std::string windowName = tab.title + "###code_graph_diagram_" + std::to_string(tab.number);
        if (!ImGui::Begin(windowName.c_str(), &tab.open))
        {
            ImGui::End();
            return;
        }

        ImGui::SetNextItemWidth(160.0f);
        ImGui::SliderFloat("Zoom", &tab.zoom, kMinZoom, kMaxZoom, "%.2fx", ImGuiSliderFlags_Logarithmic);
        StatusHint(_status, "Zoom of the diagram (Ctrl+click to type a value)");
        ImGui::SameLine();
        if (ImGui::Button("Fit"))
            tab.fitPending = true;
        StatusHint(_status, "Zoom so that the whole diagram fits the view");
        ImGui::SameLine();
        if (ImGui::Button("Copy DOT"))
        {
            ImGui::SetClipboardText(tab.dot.c_str());
            _status.Set(StatusSink::Level::kInfo, "Code graph: DOT text of " + tab.title + " copied to the clipboard");
        }
        StatusHint(_status, "Copy the DOT text of this diagram, to check it or render it elsewhere "
                            "(the Diagram panel, Graphviz)");
        ImGui::Separator();

        ImGui::BeginChild("##canvas", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None,
                          ImGuiWindowFlags_HorizontalScrollbar);
        const ImVec2 available = ImGui::GetContentRegionAvail();
        // Fitting measures the diagram drawn at the current zoom: ImGuiDot
        // reserves its size with an item, unshifted when the pivot is 0.
        ImGuiDot::Draw(tab.state, tab.zoom, tab.fitPending ? ImVec2(0.0f, 0.0f) : ImVec2(0.5f, 0.0f));
        if (tab.fitPending)
        {
            const ImVec2 drawn = ImGui::GetItemRectSize();
            if (drawn.x > 0.0f && drawn.y > 0.0f && available.x > 0.0f && available.y > 0.0f)
            {
                const float scale = std::min(available.x / drawn.x, available.y / drawn.y);
                tab.zoom = std::clamp(tab.zoom * scale, kMinZoom, kMaxFitZoom);
                ImGui::SetScrollX(0.0f);
                ImGui::SetScrollY(0.0f);
                tab.fitPending = false;
            }
        }
        ImGui::EndChild();

        ImGui::End();
    }
}
