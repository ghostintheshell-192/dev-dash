#include "code_graph_panel.h"

#include "status_sink.h"
#include "theme.h"
#include "widgets.h"
#include "../services/class_diagram_generator.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <exception>
#include <map>
#include <optional>

#include <ImGuiDot.h>
#include <imgui.h>
// ImGuiItemFlags_MixedValue: the third state of the scope check boxes.
#include <imgui_internal.h>

namespace dev_dash::ui
{
    namespace
    {
        constexpr std::string_view kScope = "::";

        constexpr float kMinZoom = 0.1f;
        constexpr float kMaxZoom = 4.0f;
        // Fitting never enlarges a small diagram past this.
        constexpr float kMaxFitZoom = 1.5f;
        // Each notch of Ctrl+wheel multiplies the zoom by this.
        constexpr float kWheelZoomStep = 1.15f;
        // Each notch of the wheel moves the view by this. [pixel]
        constexpr float kWheelPanStep = 60.0f;
        // Space left around a fitted diagram. [pixel]
        constexpr float kFitMargin = 16.0f;
        // How much of the diagram always stays in view. [pixel]
        constexpr float kKeepInView = 40.0f;

        std::string ScopeOf(const std::string& qualifiedName)
        {
            const std::size_t pos = qualifiedName.rfind(kScope);
            return pos == std::string::npos ? std::string() : qualifiedName.substr(0, pos);
        }

        // "a::b::c" → {"a", "b", "c"}; "" → {}.
        std::vector<std::string> SplitScope(const std::string& scope)
        {
            std::vector<std::string> parts;
            std::size_t start = 0;
            while (start < scope.size())
            {
                const std::size_t end = scope.find(kScope, start);
                parts.push_back(scope.substr(start, end == std::string::npos ? std::string::npos : end - start));
                if (end == std::string::npos)
                    break;
                start = end + kScope.size();
            }
            return parts;
        }

        // directory is inside container, or is container.
        bool IsWithin(const std::filesystem::path& directory, const std::filesystem::path& container)
        {
            const auto [end, unused] =
                std::mismatch(container.begin(), container.end(), directory.begin(), directory.end());
            return end == container.end();
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
        ImVec2                 pan;                   // top left corner of the diagram in the canvas [pixel]
        float                  drawnZoom    = 0.0f;   // the zoom of the last frame drawn, 0 before the first
        bool                   panning      = false;  // the view follows the mouse, dragged with a button down
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
        std::optional<std::vector<std::filesystem::path>> excluded;
        if (_excludedChosen)
            excluded.emplace(_excluded.begin(), _excluded.end());
        _reading = std::async(std::launch::async,
                              [&extractor = _extractor, root = _project.path, stop = _stopReading.get_token(),
                               excluded = std::move(excluded)] { return extractor.Extract(root, stop, excluded); });
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

        // The first reading tells which folders the default rule left out.
        if (!_excludedChosen)
        {
            _excluded.insert(_result.excludedDirectories.begin(), _result.excludedDirectories.end());
            _excludedChosen = true;
        }
        _directories = DirectoryNode{};
        for (const std::filesystem::path& directory : _result.sourceDirectories)
        {
            DirectoryNode* node = &_directories;
            std::filesystem::path relative;
            for (const std::filesystem::path& part : directory)
            {
                relative /= part;
                node           = &node->children[part.string()];
                node->relative = relative;
            }
        }

        // The tree of the scopes; drop the checked classes that no longer
        // exist.
        _tree = ScopeNode{};
        std::set<std::string> names;
        for (std::size_t i = 0; i < _result.model.classes.size(); ++i)
        {
            const std::string& name = _result.model.classes[i].qualifiedName;
            ScopeNode* node = &_tree;
            for (const std::string& part : SplitScope(ScopeOf(name)))
                node = &node->children[part];
            node->classes.push_back(i);
            names.insert(name);
        }
        const auto sortClasses = [&](auto& self, ScopeNode& node) -> void
        {
            std::sort(node.classes.begin(), node.classes.end(), [&](std::size_t a, std::size_t b)
                      { return _result.model.classes[a].qualifiedName < _result.model.classes[b].qualifiedName; });
            for (auto& [name, child] : node.children)
                self(self, child);
        };
        sortClasses(sortClasses, _tree);
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

    void CodeGraphPanel::RenderSidebarSection()
    {
        const Theme& t       = CurrentTheme();
        const bool   reading = _reading.valid();

        // ----- Reading

        // The activity goes to the status bar; an item under the mouse
        // overrides it there while hovered.
        if (reading)
            _status.SetHint("Code graph: reading the C++ classes of the project in background...");

        // The folders chosen differ from those of the classes listed.
        const bool foldersChanged =
            _hasResult
            && _excluded != std::set<std::filesystem::path>(_result.excludedDirectories.begin(),
                                                            _result.excludedDirectories.end());

        ImGui::BeginDisabled(reading);
        if (foldersChanged)
            ImGui::PushStyleColor(ImGuiCol_Text, t.accent);
        if (ImGui::Button(reading ? "Reading..." : _hasResult ? "Read again" : "Read the code"))
            StartReading();
        if (foldersChanged)
            ImGui::PopStyleColor();
        ImGui::EndDisabled();
        StatusHint(_status, foldersChanged
                                ? "The folders to read changed: read again to list the classes they hold"
                                : "Read the C++ classes of the project, skipping hidden and build directories "
                                  "(.git, build, cmake-build-*, out, node_modules) and the folders left out below");
        if (_hasResult && !reading)
        {
            ImGui::SameLine();
            ImGui::TextDisabled("%zu classes, %d files", _result.model.classes.size(), _result.filesRead);
        }
        if (_hasResult)
            RenderFolders();

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
        // The count rides on the button: the sidebar is too narrow for a
        // third item on this row.
        const std::string createLabel = _selection.empty()
                                            ? std::string("Create diagram###create")
                                            : "Create diagram (" + std::to_string(_selection.size()) + ")###create";
        if (ImGui::Button(createLabel.c_str()))
            CreateDiagram();
        StatusHint(_status, _selection.empty() ? "Check at least one class to create a diagram"
                                               : "Open the diagram of the checked classes in a new tab, "
                                                 "with the options below");
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
            _selection.clear();
        StatusHint(_status, "Uncheck all the classes, also those hidden by the filter");
        ImGui::EndDisabled();
        ImGui::Checkbox("Neighbours", &_showNeighbours);
        StatusHint(_status, "Also draw the classes one step away from the checked ones, dimmed and "
                            "with their name only");
        ImGui::SameLine();
        ImGui::Checkbox("All members", &_allMembers);
        StatusHint(_status, "Show the protected and private members too, not only the public ones");

        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint("##filter", "Filter classes", _filter.data(), _filter.size());
        StatusHint(_status, "Show only the classes whose full name contains this text (case ignored)");

        // ----- Classes by namespace

        RenderScope(_tree, std::string(), 0);
    }

    // The folders of the project holding C++ files, with a box each: the
    // unchecked ones are left out of the next reading, with all they hold.
    void CodeGraphPanel::RenderFolders()
    {
        if (_directories.children.empty())
            return;

        const std::size_t total = _result.sourceDirectories.size();
        const std::size_t read  = static_cast<std::size_t>(
            std::count_if(_result.sourceDirectories.begin(), _result.sourceDirectories.end(),
                          [&](const std::filesystem::path& directory) { return !IsExcluded(directory); }));
        const bool expanded = ImGui::TreeNode("##folders", "Folders: %zu of %zu read", read, total);
        StatusHint(_status, "The folders of the project holding C++ code: uncheck one to leave it out of the next "
                            "reading. The first reading leaves out the folders named test and tests.");
        if (expanded)
        {
            RenderDirectory(_directories, false);
            ImGui::TreePop();
        }
    }

    void CodeGraphPanel::RenderDirectory(const DirectoryNode& node, bool parentExcluded)
    {
        for (const auto& [name, child] : node.children)
        {
            ImGui::PushID(name.c_str());

            // Checked when read; mixed when read but some folder inside is
            // not. Inside an unchecked folder the boxes are disabled: the
            // folder above decides.
            const bool excluded = parentExcluded || _excluded.contains(child.relative);
            bool       checked  = !excluded;
            const bool mixed    = !excluded && HasExcludedBelow(child.relative);
            ImGui::BeginDisabled(parentExcluded);
            ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, mixed);
            if (ImGui::Checkbox("##read", &checked))
                SetExcluded(child.relative, !checked);
            ImGui::PopItemFlag();
            ImGui::EndDisabled();
            StatusHint(_status, parentExcluded ? child.relative.string() + ": inside a folder left out"
                                               : child.relative.string() + ": check to read it, uncheck to leave it "
                                                                           "out with all it holds");
            ImGui::SameLine();

            const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth
                                             | (child.children.empty() ? ImGuiTreeNodeFlags_Leaf : 0);
            if (ImGui::TreeNodeEx(name.c_str(), flags))
            {
                RenderDirectory(child, excluded);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }

    bool CodeGraphPanel::IsExcluded(const std::filesystem::path& relative) const
    {
        return std::any_of(_excluded.begin(), _excluded.end(),
                           [&](const std::filesystem::path& excluded) { return IsWithin(relative, excluded); });
    }

    bool CodeGraphPanel::HasExcludedBelow(const std::filesystem::path& relative) const
    {
        return std::any_of(_excluded.begin(), _excluded.end(), [&](const std::filesystem::path& excluded)
                           { return excluded != relative && IsWithin(excluded, relative); });
    }

    // Excluding or including a folder settles the folders inside it too.
    void CodeGraphPanel::SetExcluded(const std::filesystem::path& relative, bool excluded)
    {
        std::erase_if(_excluded, [&](const std::filesystem::path& other) { return IsWithin(other, relative); });
        if (excluded)
            _excluded.insert(relative);
    }

    void CodeGraphPanel::CollectVisible(const ScopeNode& node, std::vector<std::size_t>& visible) const
    {
        const std::string_view filter(_filter.data());
        for (const std::size_t index : node.classes)
            if (Matches(_result.model.classes[index].qualifiedName, filter))
                visible.push_back(index);
        for (const auto& [name, child] : node.children)
            CollectVisible(child, visible);
    }

    // The scopes nested in node, then its own classes. A scope with no class
    // shown by the filter is left out. A class with nested classes is one
    // node: its box covers the class and the classes nested in it.
    void CodeGraphPanel::RenderScope(const ScopeNode& node, const std::string& path, int depth)
    {
        const bool             filtering = _filter[0] != '\0';
        const std::string_view filter(_filter.data());

        // The class of node named like a nested scope, if any.
        const auto classNamed = [&](const std::string& name) -> std::optional<std::size_t>
        {
            for (const std::size_t index : node.classes)
                if (ShortNameOf(_result.model.classes[index].qualifiedName) == name)
                    return index;
            return std::nullopt;
        };
        std::set<std::size_t> classesAsScopes;

        for (const auto& [name, child] : node.children)
        {
            const std::optional<std::size_t> ownClass = classNamed(name);
            if (ownClass)
                classesAsScopes.insert(*ownClass);

            std::vector<std::size_t> visible;
            CollectVisible(child, visible);
            const std::size_t nestedCount = visible.size();
            if (ownClass && Matches(_result.model.classes[*ownClass].qualifiedName, filter))
                visible.push_back(*ownClass);
            if (visible.empty())
                continue;

            const std::string childPath = path.empty() ? name : path + std::string(kScope) + name;
            ImGui::PushID(name.c_str());

            // One box for the whole scope, nested scopes included: checked
            // when all its classes shown by the filter are, mixed when some
            // are; a click checks them all, or clears them when all are.
            const std::size_t checkedCount = static_cast<std::size_t>(
                std::count_if(visible.begin(), visible.end(), [&](std::size_t index)
                              { return _selection.contains(_result.model.classes[index].qualifiedName); }));
            bool       all   = checkedCount == visible.size();
            const bool mixed = checkedCount > 0 && !all;
            ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, mixed);
            const bool changed = ImGui::Checkbox("##all", &all);
            ImGui::PopItemFlag();
            StatusHint(_status, "Check or uncheck " + std::string(ownClass ? "the class " : "every class of ") + childPath
                                    + (ownClass ? " and the classes nested in it" : ", nested scopes included")
                                    + " shown by the filter");
            if (changed)
                for (const std::size_t index : visible)
                {
                    const std::string& qualifiedName = _result.model.classes[index].qualifiedName;
                    if (all)
                        _selection.insert(qualifiedName);
                    else
                        _selection.erase(qualifiedName);
                }
            ImGui::SameLine();

            // The outer namespaces start open; while filtering, every scope
            // with a match opens to show it.
            if (filtering)
                ImGui::SetNextItemOpen(true);
            const ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_SpanAvailWidth | (depth == 0 ? ImGuiTreeNodeFlags_DefaultOpen : 0);
            const bool open = ImGui::TreeNodeEx(name.c_str(), flags);
            if (ownClass)
            {
                const core::CodeClass& cls = _result.model.classes[*ownClass];
                StatusHint(_status, cls.qualifiedName + "  ·  " + cls.file.lexically_relative(_project.path).string()
                                        + "  ·  " + std::to_string(cls.members.size()) + " members, "
                                        + std::to_string(nestedCount) + " nested classes");
            }
            else
                StatusHint(_status, childPath + "  ·  " + std::to_string(visible.size()) + " classes");
            if (open)
            {
                RenderScope(child, childPath, depth + 1);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        for (const std::size_t index : node.classes)
            if (!classesAsScopes.contains(index) && Matches(_result.model.classes[index].qualifiedName, filter))
                RenderClass(index);
    }

    void CodeGraphPanel::RenderClass(std::size_t index)
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
        StatusHint(_status, cls.qualifiedName + "  ·  " + cls.file.lexically_relative(_project.path).string() + "  ·  "
                                + std::to_string(cls.members.size()) + " members");
        ImGui::PopID();
    }

    void CodeGraphPanel::RenderDiagrams(ImGuiID dockspaceId)
    {
        CollectReading();

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

    // Ctrl+wheel zooms around the point under the mouse, the wheel moves the
    // view up and down (with Shift, left and right), dragging with the left
    // or middle button moves it anywhere. Called before the diagram is drawn,
    // so the new view is drawn in this frame.
    void CodeGraphPanel::HandleViewInput(DiagramTab& tab, const ImVec2& origin)
    {
        const ImGuiIO& io      = ImGui::GetIO();
        const bool     hovered = ImGui::IsWindowHovered();

        if (hovered && io.MouseWheel != 0.0f)
        {
            if (io.KeyCtrl)
            {
                const float newZoom =
                    std::clamp(tab.zoom * std::pow(kWheelZoomStep, io.MouseWheel), kMinZoom, kMaxZoom);
                // The point of the diagram under the mouse stays there.
                const float ratio = newZoom / tab.zoom;
                const ImVec2 mouse(io.MousePos.x - origin.x, io.MousePos.y - origin.y);
                tab.pan  = ImVec2(mouse.x - (mouse.x - tab.pan.x) * ratio, mouse.y - (mouse.y - tab.pan.y) * ratio);
                tab.zoom = newZoom;
            }
            else if (io.KeyShift)
                tab.pan.x += io.MouseWheel * kWheelPanStep;
            else
                tab.pan.y += io.MouseWheel * kWheelPanStep;
        }
        if (hovered && io.MouseWheelH != 0.0f)
            tab.pan.x += io.MouseWheelH * kWheelPanStep;

        if (hovered && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)))
            tab.panning = true;
        if (tab.panning)
        {
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsMouseDown(ImGuiMouseButton_Middle))
                tab.panning = false;
            else
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                tab.pan = ImVec2(tab.pan.x + io.MouseDelta.x, tab.pan.y + io.MouseDelta.y);
            }
        }
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
        StatusHint(_status, "Zoom of the diagram (Ctrl+click to type a value). On the diagram: Ctrl+wheel zooms "
                            "around the mouse; dragging, the wheel and Shift+wheel move the view");
        ImGui::SameLine();
        if (ImGui::Button("Fit"))
            tab.fitPending = true;
        StatusHint(_status, "Zoom so that the whole diagram fits the view, and centre it");
        ImGui::SameLine();
        if (ImGui::Button("Copy DOT"))
        {
            ImGui::SetClipboardText(tab.dot.c_str());
            _status.Set(StatusSink::Level::kInfo, "Code graph: DOT text of " + tab.title + " copied to the clipboard");
        }
        StatusHint(_status, "Copy the DOT text of this diagram, to check it or render it elsewhere "
                            "(the Diagram panel, Graphviz)");
        ImGui::Separator();

        // The canvas has no scroll bars: the diagram is drawn at pan from
        // the top left corner, so it can go anywhere, also when smaller than
        // the view, and zooming keeps the point under the mouse in place.
        ImGui::BeginChild("##canvas", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        const ImVec2 origin    = ImGui::GetCursorScreenPos();
        const ImVec2 available = ImGui::GetContentRegionAvail();
        // A zoom changed by the slider keeps the centre of the view in place.
        if (tab.drawnZoom > 0.0f && tab.zoom != tab.drawnZoom)
        {
            const float  ratio = tab.zoom / tab.drawnZoom;
            const ImVec2 centre(available.x / 2.0f, available.y / 2.0f);
            tab.pan = ImVec2(centre.x - (centre.x - tab.pan.x) * ratio, centre.y - (centre.y - tab.pan.y) * ratio);
        }
        HandleViewInput(tab, origin);

        ImGui::SetCursorScreenPos(ImVec2(origin.x + tab.pan.x, origin.y + tab.pan.y));
        ImGuiDot::Draw(tab.state, tab.zoom);
        const ImVec2 drawn = ImGui::GetItemRectSize();

        // Fitting measures the diagram drawn at the current zoom, then
        // centres it: its size grows with the zoom.
        if (tab.fitPending && drawn.x > 0.0f && drawn.y > 0.0f && available.x > 0.0f && available.y > 0.0f)
        {
            const ImVec2 room(available.x - 2.0f * kFitMargin, available.y - 2.0f * kFitMargin);
            const float  newZoom =
                std::clamp(tab.zoom * std::min(room.x / drawn.x, room.y / drawn.y), kMinZoom, kMaxFitZoom);
            const float ratio = newZoom / tab.zoom;
            tab.zoom          = newZoom;
            tab.pan           = ImVec2((available.x - drawn.x * ratio) / 2.0f, (available.y - drawn.y * ratio) / 2.0f);
            tab.fitPending    = false;
        }
        else
        {
            // Keep a corner of the diagram in view: it cannot be lost by
            // dragging it out.
            tab.pan.x = std::clamp(tab.pan.x, kKeepInView - drawn.x, available.x - kKeepInView);
            tab.pan.y = std::clamp(tab.pan.y, kKeepInView - drawn.y, available.y - kKeepInView);
        }
        tab.drawnZoom = tab.zoom;
        ImGui::EndChild();

        ImGui::End();
    }
}
