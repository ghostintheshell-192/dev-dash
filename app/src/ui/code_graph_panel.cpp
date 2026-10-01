#include "code_graph_panel.h"

#include "theme.h"
#include "../services/class_diagram_generator.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <exception>
#include <map>

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

    CodeGraphPanel::CodeGraphPanel(services::CppClassExtractor& extractor,
                                   services::ClassDiagramGenerator& generator,
                                   const core::Project& project)
        : _extractor(extractor)
        , _generator(generator)
        , _project(project)
    {
    }

    // A reading still running is asked to stop, then waited for by the
    // future's destructor: the worker stops at the next file.
    CodeGraphPanel::~CodeGraphPanel()
    {
        _stopReading.request_stop();
        ImGuiDot::CleanUp(_diagram);
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
            return;
        }

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

        _dot = _generator.Generate(_result.model, _selection, options);
        ImGuiDot::Update(_diagram, _dot);
        _fitPending = true;
    }

    void CodeGraphPanel::Render(bool* open)
    {
        CollectReading();

        if (!ImGui::Begin("Code graph", open))
        {
            ImGui::End();
            return;
        }

        ImGui::BeginChild("##code_graph_classes", ImVec2(_listWidth, 0.0f), ImGuiChildFlags_ResizeX);
        RenderClassList();
        _listWidth = ImGui::GetWindowWidth();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##code_graph_diagram", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None);
        RenderDiagram();
        ImGui::EndChild();

        ImGui::End();
    }

    void CodeGraphPanel::RenderClassList()
    {
        const Theme& t       = CurrentTheme();
        const bool   reading = _reading.valid();

        // ----- Reading

        ImGui::BeginDisabled(reading);
        if (ImGui::Button(_hasResult ? "Read again" : "Read the code"))
            StartReading();
        ImGui::EndDisabled();
        if (reading)
        {
            ImGui::SameLine();
            ImGui::TextDisabled("Reading...");
        }
        else if (_hasResult)
        {
            ImGui::SameLine();
            ImGui::TextDisabled("%zu classes, %d files", _result.model.classes.size(), _result.filesRead);
        }

        if (!_readError.empty())
            ImGui::TextColored(t.removed, "%s", _readError.c_str());
        if (_hasResult && !_result.partlyReadFiles.empty())
        {
            ImGui::TextColored(t.modified, "%zu files read in part (?)", _result.partlyReadFiles.size());
            if (ImGui::BeginItemTooltip())
            {
                ImGui::TextUnformatted("Code the reader did not understand: macros it cannot expand,\n"
                                       "syntax its grammar misses, or real errors. The rest of each\n"
                                       "file is read; a class inside the unread part is missing.");
                ImGui::Separator();
                for (const std::filesystem::path& file : _result.partlyReadFiles)
                    ImGui::TextDisabled("%s", file.lexically_relative(_project.path).string().c_str());
                ImGui::EndTooltip();
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
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
            _selection.clear();
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextDisabled("%zu checked", _selection.size());
        ImGui::Checkbox("Neighbours", &_showNeighbours);
        ImGui::SameLine();
        ImGui::Checkbox("All members", &_allMembers);

        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint("##filter", "Filter classes", _filter.data(), _filter.size());
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
            if (ImGui::Checkbox("##all", &all))
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
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                        ImGui::SetTooltip("%s\n%s\n%zu members", cls.qualifiedName.c_str(),
                                          cls.file.lexically_relative(_project.path).string().c_str(),
                                          cls.members.size());
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    void CodeGraphPanel::RenderDiagram()
    {
        ImGui::SetNextItemWidth(160.0f);
        ImGui::SliderFloat("Zoom", &_zoom, kMinZoom, kMaxZoom, "%.2fx", ImGuiSliderFlags_Logarithmic);
        ImGui::SameLine();
        ImGui::BeginDisabled(_dot.empty());
        if (ImGui::Button("Fit"))
            _fitPending = true;
        ImGui::SameLine();
        if (ImGui::Button("Copy DOT"))
            ImGui::SetClipboardText(_dot.c_str());
        ImGui::EndDisabled();
        ImGui::Separator();

        if (_dot.empty())
        {
            ImGui::TextDisabled("Check some classes, then Create diagram.");
            return;
        }

        ImGui::BeginChild("##code_graph_canvas", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None,
                          ImGuiWindowFlags_HorizontalScrollbar);
        const ImVec2 available = ImGui::GetContentRegionAvail();
        // Fitting measures the diagram drawn at the current zoom: ImGuiDot
        // reserves its size with an item, unshifted when the pivot is 0.
        ImGuiDot::Draw(_diagram, _zoom, _fitPending ? ImVec2(0.0f, 0.0f) : ImVec2(0.5f, 0.0f));
        if (_fitPending)
        {
            const ImVec2 drawn = ImGui::GetItemRectSize();
            if (drawn.x > 0.0f && drawn.y > 0.0f)
            {
                const float scale = std::min(available.x / drawn.x, available.y / drawn.y);
                _zoom = std::clamp(_zoom * scale, kMinZoom, kMaxFitZoom);
                ImGui::SetScrollX(0.0f);
                ImGui::SetScrollY(0.0f);
            }
            _fitPending = false;
        }
        ImGui::EndChild();
    }
}
