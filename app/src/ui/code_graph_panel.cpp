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
#include <ctime>
#include <exception>
#include <map>
#include <optional>

#include <ImGuiDot.h>
#include <imgui.h>
#include <imgui_internal.h>   // ImGuiItemFlags_MixedValue: the third state of the check boxes

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
        // Width of the filters pane of a diagram, in font sizes: at first,
        // and the narrowest it can be dragged to.
        constexpr float kFiltersWidthEm    = 16.0f;
        constexpr float kFiltersMinWidthEm = 8.0f;
        // The share of the tab the filters pane can take at most.
        constexpr float kFiltersMaxShare = 0.7f;
        // The bar between the diagram and its filters, dragged to resize them. [pixel]
        constexpr float kSplitterWidth = 4.0f;

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

        // An entry of the section that executes when clicked: a leaf of the
        // tree, so its label lines up with the entries that open.
        bool ActionEntry(const char* label)
        {
            ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen
                                         | ImGuiTreeNodeFlags_SpanAvailWidth);
            return ImGui::IsItemClicked(ImGuiMouseButton_Left);
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

    // One diagram, in a tab of its own: its selection and its filters, the
    // DOT text made from them (kept for Copy DOT) and the state ImGuiDot laid
    // out from it.
    struct CodeGraphPanel::DiagramTab
    {
        int                    number = 0;
        std::string            title;           // shown on the tab
        std::string            dot;
        ImGuiDot::DiagramState state;
        // What the diagram shows: its classes and its options. The filters
        // and the toolbar of the tab change them, and the diagram follows.
        std::set<std::string>        classes;
        bool                         showNeighbours = true;
        std::set<core::MemberAccess> shownAccess    = {core::MemberAccess::kPublic};
        // The filters pane: shown, its search text, how it lists the classes.
        bool                   showFilters  = true;
        float                  filtersWidth = 0.0f;   // [pixel], 0 until the first frame
        std::array<char, 128>  filter{};
        ClassView              view = ClassView::kNamespaces;
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

    void CodeGraphPanel::StartScan()
    {
        _scanning = std::async(std::launch::async,
                               [&extractor = _extractor, root = _project.path] { return extractor.Scan(root); });
    }

    void CodeGraphPanel::CollectScan()
    {
        if (!_scanning.valid() || _scanning.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
            return;
        services::SourceScan scan;
        try
        {
            scan = _scanning.get();
        }
        catch (const std::exception& e)
        {
            _readError = std::string("Finding the folders failed: ") + e.what();
            return;
        }
        _scanned = true;
        if (!_excludedChosen)
        {
            _excluded.insert(scan.excludedDirectories.begin(), scan.excludedDirectories.end());
            _excludedChosen = true;
        }
        if (!_hasResult)
            SetSourceDirectories(scan.sourceDirectories);
    }

    // The tree of the folders to read.
    void CodeGraphPanel::SetSourceDirectories(const std::vector<std::filesystem::path>& directories)
    {
        _sourceDirectories = directories;
        _directories       = DirectoryNode{};
        for (const std::filesystem::path& directory : directories)
        {
            DirectoryNode*        node = &_directories;
            std::filesystem::path relative;
            for (const std::filesystem::path& part : directory)
            {
                relative /= part;
                node           = &node->children[part.string()];
                node->relative = relative;
            }
        }
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
            _readError = std::string("Analysis failed: ") + e.what();
            _status.Set(StatusSink::Level::kError, "Code graph: " + _readError);
            return;
        }
        _status.Set(StatusSink::Level::kInfo,
                    "Code graph: " + std::to_string(_result.model.classes.size()) + " classes read from "
                        + std::to_string(_result.filesRead.size()) + " files");
        const std::time_t now = std::time(nullptr);
        char              readAt[8];
        std::strftime(readAt, sizeof(readAt), "%H:%M", std::localtime(&now));
        _readAt = readAt;

        // The folders may have changed since they were found.
        if (!_excludedChosen)
        {
            _excluded.insert(_result.excludedDirectories.begin(), _result.excludedDirectories.end());
            _excludedChosen = true;
        }
        SetSourceDirectories(_result.sourceDirectories);

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
        // The same classes by the folder of their file.
        _folderTree = ScopeNode{};
        for (std::size_t i = 0; i < _result.model.classes.size(); ++i)
        {
            ScopeNode* node = &_folderTree;
            for (const std::filesystem::path& part :
                 _result.model.classes[i].file.parent_path().lexically_relative(_project.path))
                if (part != ".")
                    node = &node->children[part.string()];
            node->classes.push_back(i);
        }
        const auto sortClasses = [&](auto& self, ScopeNode& node) -> void
        {
            std::sort(node.classes.begin(), node.classes.end(), [&](std::size_t a, std::size_t b)
                      { return _result.model.classes[a].qualifiedName < _result.model.classes[b].qualifiedName; });
            for (auto& [name, child] : node.children)
                self(self, child);
        };
        sortClasses(sortClasses, _tree);
        sortClasses(sortClasses, _folderTree);

        // The open diagrams follow the new reading: the classes that no
        // longer exist leave their selection.
        for (const std::unique_ptr<DiagramTab>& tab : _tabs)
        {
            std::erase_if(tab->classes, [&](const std::string& name) { return !names.contains(name); });
            GenerateDiagram(*tab);
        }
    }

    // An empty diagram, with its filters open: what is checked there is drawn.
    void CodeGraphPanel::CreateDiagram()
    {
        auto tab            = std::make_unique<DiagramTab>();
        tab->number         = _nextTabNumber++;
        tab->showNeighbours = _showNeighbours;
        tab->shownAccess    = _shownAccess;
        GenerateDiagram(*tab);
        _status.Set(StatusSink::Level::kInfo, "Code graph: " + tab->title + " opened: check the classes to draw");
        _tabs.push_back(std::move(tab));
    }

    // The DOT text of the tab from its classes and options, laid out again,
    // and the title that says what it shows.
    void CodeGraphPanel::GenerateDiagram(DiagramTab& tab)
    {
        // "Diagram 3: Shell, Sidebar +4": the first checked names say what
        // the diagram is about.
        constexpr std::size_t kNamesInTitle = 2;
        std::string names;
        std::size_t count = 0;
        for (const std::string& name : tab.classes)
        {
            if (count++ < kNamesInTitle)
                names += (names.empty() ? "" : ", ") + ShortNameOf(name);
        }
        if (count > kNamesInTitle)
            names += " +" + std::to_string(count - kNamesInTitle);
        tab.title = "Diagram " + std::to_string(tab.number) + (names.empty() ? std::string() : ": " + names);

        const Theme& t = CurrentTheme();
        services::DiagramOptions options;
        options.showNeighbours = tab.showNeighbours;
        options.shownAccess    = tab.shownAccess;
        // ImGuiDot does not draw records yet: plain boxes, one line per member.
        options.recordShapes        = false;
        options.palette.classFill   = ToHex(t.panelBg);
        options.palette.classBorder = ToHex(t.accent);
        options.palette.text        = ToHex(t.text);
        options.palette.ghostBorder = ToHex(t.textDim);
        options.palette.ghostText   = ToHex(t.textDim);
        options.palette.edge        = ToHex(t.textDim);

        tab.dot = _generator.Generate(_result.model, tab.classes, options);
        ImGuiDot::Update(tab.state, tab.dot);
        // The layout starts over with every change: fit the new diagram, or
        // part of it falls outside the view.
        tab.fitPending = true;
    }

    // The changed folders: those chosen differ from those of the classes
    // listed, a new reading is due.
    bool CodeGraphPanel::FoldersChanged() const
    {
        return _hasResult
            && _excluded != std::set<std::filesystem::path>(_result.excludedDirectories.begin(),
                                                            _result.excludedDirectories.end());
    }

    // Opening the tab the first time finds the folders, to choose before
    // the first analysis.
    void CodeGraphPanel::ShowAnalysis(bool show)
    {
        _analysisOpen  = show;
        _analysisFocus = show;
        if (show && !_scanned && !_scanning.valid() && !_hasResult)
            StartScan();
    }

    // The section holds commands: New code analysis, New diagram. An entry
    // not available is greyed out, with the reason in the status bar.
    void CodeGraphPanel::RenderSidebarSection()
    {
        const bool reading = _reading.valid();

        // The activity goes to the status bar; an item under the mouse
        // overrides it there while hovered.
        if (reading)
            _status.SetHint("Code graph: analyzing the C++ code of the project in background...");

        // ----- New code analysis: the tab where the folders are chosen and
        // the analysis is started.

        if (ActionEntry("New code analysis"))
            ShowAnalysis(true);
        StatusHint(_status, reading ? "Open the code analysis: analyzing the C++ code of the project..."
                                    : "Open the code analysis: choose the folders to read, then Analyze");

        // ----- New diagram

        ImGui::BeginDisabled(!_hasResult);
        if (ActionEntry("New diagram"))
            CreateDiagram();
        ImGui::EndDisabled();
        StatusHint(_status, _hasResult ? "Open an empty diagram in a new tab, with its filters: the classes checked "
                                         "there are drawn"
                                       : "Available after analyzing the code");
    }

    // The code analysis: Analyze with what it last read, the folders to
    // read, open, then the files read and those read in part, once read.
    void CodeGraphPanel::RenderAnalysisTab()
    {
        if (!ImGui::Begin("Code analysis", &_analysisOpen))
        {
            ImGui::End();
            return;
        }

        const Theme& t              = CurrentTheme();
        const bool   reading        = _reading.valid();
        const bool   scanning       = _scanning.valid();
        const bool   foldersChanged = FoldersChanged();

        ImGui::BeginDisabled(reading || scanning);
        if (foldersChanged)
            ImGui::PushStyleColor(ImGuiCol_Text, t.accent);
        if (ImGui::Button("Analyze"))
            StartReading();
        if (foldersChanged)
            ImGui::PopStyleColor();
        ImGui::EndDisabled();
        StatusHint(_status, reading          ? "Analyzing the C++ code of the project..."
                            : foldersChanged ? "Analyze: the folders to read changed since the last analysis"
                                             : "Read the C++ code of the folders checked below");
        ImGui::SameLine();
        if (reading)
            ImGui::TextDisabled("Reading the C++ files of the project...");
        else if (_hasResult)
            ImGui::TextDisabled("%zu files, %zu classes, read at %s", _result.filesRead.size(),
                                _result.model.classes.size(), _readAt.c_str());
        else
            ImGui::TextDisabled("Not analyzed yet: check the folders to read, then Analyze.");
        if (!_readError.empty())
            ImGui::TextColored(t.removed, "%s", _readError.c_str());

        ImGui::BeginChild("##analysis");
        RenderFolders();
        if (_hasResult)
        {
            ImGui::Spacing();
            RenderFilesRead();
            if (!_result.partlyReadFiles.empty())
                RenderPartlyRead();
        }
        ImGui::EndChild();
        ImGui::End();
    }

    // The files read, with their path in the project.
    void CodeGraphPanel::RenderFilesRead()
    {
        const bool open = ImGui::CollapsingHeader(
            ("Files read (" + std::to_string(_result.filesRead.size()) + ")###files_read").c_str());
        StatusHint(_status, "The C++ files read, from the folders checked above; hidden and build directories are "
                            "always skipped");
        if (!open)
            return;
        ImGui::Indent();
        for (const std::filesystem::path& file : _result.filesRead)
            ImGui::TextUnformatted(file.lexically_relative(_project.path).string().c_str());
        ImGui::Unindent();
    }

    // The files read in part, one row each: where the reader stopped
    // understanding, why, and the line of code.
    void CodeGraphPanel::RenderPartlyRead()
    {
        const Theme& t = CurrentTheme();
        // Neutral, not a warning: it tells the limits of the reader, not a
        // fault of the project. Open: it is what the analysis has to say.
        ImGui::PushStyleColor(ImGuiCol_Text, t.info);
        const bool open = ImGui::CollapsingHeader(
            ("Read in part (" + std::to_string(_result.partlyReadFiles.size()) + ")###partly_read").c_str(),
            ImGuiTreeNodeFlags_DefaultOpen);
        ImGui::PopStyleColor();
        StatusHint(_status, "Code the reader did not understand, the first place in each file: usually a macro "
                            "(it has no preprocessor) or syntax its grammar misses. The rest is read.");
        if (!open)
            return;

        constexpr ImGuiTableFlags kFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg
                                           | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;
        if (!ImGui::BeginTable("##partly_read_table", 4, kFlags))
            return;
        ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthStretch, 3.0f);
        ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Not understood", ImGuiTableColumnFlags_WidthStretch, 3.0f);
        ImGui::TableSetupColumn("Code", ImGuiTableColumnFlags_WidthStretch, 4.0f);
        ImGui::TableHeadersRow();
        for (const services::PartlyReadFile& partly : _result.partlyReadFiles)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(partly.file.lexically_relative(_project.path).string().c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%d", partly.line);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(partly.reason.c_str());
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Text, t.textDim);
            ImGui::TextUnformatted(partly.text.c_str());
            ImGui::PopStyleColor();
        }
        ImGui::EndTable();
    }

    // The folders of the project holding C++ files, with a box each: the
    // unchecked ones are left out of the next analysis, with all they hold.
    void CodeGraphPanel::RenderFolders()
    {
        const std::size_t total = _sourceDirectories.size();
        const std::size_t read  = static_cast<std::size_t>(
            std::count_if(_sourceDirectories.begin(), _sourceDirectories.end(),
                          [&](const std::filesystem::path& directory) { return !IsExcluded(directory); }));
        ImGui::SeparatorText(("Folders to read (" + std::to_string(read) + " of " + std::to_string(total) + ")").c_str());
        StatusHint(_status, "The folders holding C++ code: uncheck those to leave out, then Analyze. Names are "
                            "linked to classes only among the files read");
        if (_scanning.valid())
            ImGui::TextDisabled("Finding the folders...");
        else if (_directories.children.empty())
            ImGui::TextDisabled("No folder holds C++ files.");
        else
            RenderDirectory(_directories, false);
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

            // All open: the whole tree is the choice.
            const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen
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

    void CodeGraphPanel::CollectVisible(const DiagramTab& tab, const ScopeNode& node,
                                        std::vector<std::size_t>& visible) const
    {
        const std::string_view filter(tab.filter.data());
        for (const std::size_t index : node.classes)
            if (Matches(_result.model.classes[index].qualifiedName, filter))
                visible.push_back(index);
        for (const auto& [name, child] : node.children)
            CollectVisible(tab, child, visible);
    }

    // The scopes nested in node, then its own classes. A scope with no class
    // shown by the filter is left out. A class with nested classes is one
    // node: its box covers the class and the classes nested in it.
    bool CodeGraphPanel::RenderScope(DiagramTab& tab, const ScopeNode& node, const std::string& path, int depth)
    {
        const bool             filtering = tab.filter[0] != '\0';
        const std::string_view filter(tab.filter.data());
        const bool             byFolder  = tab.view == ClassView::kFolders;
        bool                   changedAny = false;
        const std::string      separator = byFolder ? "/" : std::string(kScope);

        // The class of node named like a nested scope, if any.
        const auto classNamed = [&](const std::string& name) -> std::optional<std::size_t>
        {
            for (const std::size_t index : node.classes)
                if (ShortNameOf(_result.model.classes[index].qualifiedName) == name)
                    return index;
            return std::nullopt;
        };
        std::set<std::size_t> classesAsScopes;

        for (const auto& [childName, childNode] : node.children)
        {
            // By folder, a chain of folders with nothing but one folder each
            // is one node: "app/src" rather than "app" then "src".
            std::string      name  = childName;
            const ScopeNode* chain = &childNode;
            while (byFolder && chain->classes.empty() && chain->children.size() == 1)
            {
                name += "/" + chain->children.begin()->first;
                chain = &chain->children.begin()->second;
            }
            const ScopeNode& child = *chain;

            const std::optional<std::size_t> ownClass = byFolder ? std::nullopt : classNamed(childName);
            if (ownClass)
                classesAsScopes.insert(*ownClass);

            std::vector<std::size_t> visible;
            CollectVisible(tab, child, visible);
            const std::size_t nestedCount = visible.size();
            if (ownClass && Matches(_result.model.classes[*ownClass].qualifiedName, filter))
                visible.push_back(*ownClass);
            if (visible.empty())
                continue;

            const std::string childPath = path.empty() ? name : path + separator + name;
            ImGui::PushID(name.c_str());

            // One box for the whole scope, nested scopes included: checked
            // when all its classes shown by the filter are, mixed when some
            // are; a click checks them all, or clears them when all are.
            const std::size_t checkedCount = static_cast<std::size_t>(
                std::count_if(visible.begin(), visible.end(), [&](std::size_t index)
                              { return tab.classes.contains(_result.model.classes[index].qualifiedName); }));
            bool       all   = checkedCount == visible.size();
            const bool mixed = checkedCount > 0 && !all;
            ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, mixed);
            const bool changed = ImGui::Checkbox("##all", &all);
            ImGui::PopItemFlag();
            StatusHint(_status, "Check or uncheck " + std::string(ownClass ? "the class " : "every class of ") + childPath
                                    + (ownClass ? " and the classes nested in it"
                                                : byFolder ? ", nested folders included" : ", nested scopes included")
                                    + " shown by the filter");
            changedAny |= changed;
            if (changed)
                for (const std::size_t index : visible)
                {
                    const std::string& qualifiedName = _result.model.classes[index].qualifiedName;
                    if (all)
                        tab.classes.insert(qualifiedName);
                    else
                        tab.classes.erase(qualifiedName);
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
                changedAny |= RenderScope(tab, child, childPath, depth + 1);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        for (const std::size_t index : node.classes)
            if (!classesAsScopes.contains(index) && Matches(_result.model.classes[index].qualifiedName, filter))
                changedAny |= RenderClass(tab, index);
        return changedAny;
    }

    bool CodeGraphPanel::RenderClass(DiagramTab& tab, std::size_t index)
    {
        const core::CodeClass& cls = _result.model.classes[index];
        bool checked = tab.classes.contains(cls.qualifiedName);
        ImGui::PushID(static_cast<int>(index));
        const bool changed = ImGui::Checkbox(ShortNameOf(cls.qualifiedName).c_str(), &checked);
        if (changed)
        {
            if (checked)
                tab.classes.insert(cls.qualifiedName);
            else
                tab.classes.erase(cls.qualifiedName);
        }
        StatusHint(_status, cls.qualifiedName + "  ·  " + cls.file.lexically_relative(_project.path).string() + "  ·  "
                                + std::to_string(cls.members.size()) + " members");
        ImGui::PopID();
        return changed;
    }

    void CodeGraphPanel::RenderTabs(ImGuiID dockspaceId)
    {
        CollectScan();
        CollectReading();

        if (_analysisOpen)
        {
            ImGui::SetNextWindowDockID(dockspaceId, ImGuiCond_FirstUseEver);
            if (_analysisFocus)
            {
                ImGui::SetNextWindowFocus();
                _analysisFocus = false;
            }
            RenderAnalysisTab();
        }

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
        // The number after ### keeps the window identity when the title
        // changes with the selection.
        const std::string windowName = tab.title + "###code_graph_diagram_" + std::to_string(tab.number);
        if (!ImGui::Begin(windowName.c_str(), &tab.open))
        {
            ImGui::End();
            return;
        }

        RenderToolbar(tab);
        ImGui::Separator();

        // The filters at the right of the diagram, resized by dragging the
        // bar between them; closed, the whole width goes to the diagram.
        if (!tab.showFilters)
        {
            RenderCanvas(tab, 0.0f);
            ImGui::End();
            return;
        }
        const float available = ImGui::GetContentRegionAvail().x;
        const float minWidth  = kFiltersMinWidthEm * ImGui::GetFontSize();
        if (tab.filtersWidth <= 0.0f)
            tab.filtersWidth = kFiltersWidthEm * ImGui::GetFontSize();
        tab.filtersWidth = std::clamp(tab.filtersWidth, minWidth, std::max(minWidth, available * kFiltersMaxShare));

        RenderCanvas(tab, available - tab.filtersWidth - kSplitterWidth);
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::InvisibleButton("##filters_splitter", ImVec2(kSplitterWidth, -1.0f));
        if (ImGui::IsItemActive())
            tab.filtersWidth -= ImGui::GetIO().MouseDelta.x;
        if (ImGui::IsItemHovered() || ImGui::IsItemActive())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        StatusHint(_status, "Drag to resize the filters");
        const Theme& t = CurrentTheme();
        ImGui::GetWindowDrawList()->AddRectFilled(
            ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
            ImGui::GetColorU32(ImGui::IsItemActive() ? t.accent : ImGui::IsItemHovered() ? t.accentHover : t.border));
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::BeginChild("##filters", ImVec2(tab.filtersWidth, 0.0f), ImGuiChildFlags_AlwaysUseWindowPadding);
        RenderFilters(tab);
        ImGui::EndChild();

        ImGui::End();
    }

    // The options of the diagram, always in view: what it leaves out must
    // not go unnoticed. The last ones chosen are those of the next diagram.
    void CodeGraphPanel::RenderToolbar(DiagramTab& tab)
    {
        ImGui::Checkbox("Filters", &tab.showFilters);
        StatusHint(_status, "Show or hide the classes to check for this diagram");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(140.0f);
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
                            "(the DOT preview, Graphviz)");

        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        bool changed = false;
        if (ImGui::Checkbox("Neighbours", &tab.showNeighbours))
        {
            _showNeighbours = tab.showNeighbours;
            changed         = true;
        }
        StatusHint(_status, "Also draw the classes one step away from those of the diagram, dimmed and with "
                            "their name only");

        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextUnformatted("Members:");
        const auto accessBox = [&](const char* label, core::MemberAccess access, const char* hint)
        {
            ImGui::SameLine();
            bool shown = tab.shownAccess.contains(access);
            if (ImGui::Checkbox(label, &shown))
            {
                if (shown)
                    tab.shownAccess.insert(access);
                else
                    tab.shownAccess.erase(access);
                _shownAccess = tab.shownAccess;
                changed      = true;
            }
            StatusHint(_status, hint);
        };
        accessBox("public", core::MemberAccess::kPublic, "Show the public members in the class boxes");
        accessBox("protected", core::MemberAccess::kProtected, "Show the protected members in the class boxes");
        accessBox("private", core::MemberAccess::kPrivate, "Show the private members in the class boxes");

        if (changed)
            GenerateDiagram(tab);
    }

    // The classes analyzed, to check for this diagram: search, view, Clear
    // selection, the tree. Every change draws the diagram again.
    void CodeGraphPanel::RenderFilters(DiagramTab& tab)
    {
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint("##filter", "Name contains...", tab.filter.data(), tab.filter.size());
        StatusHint(_status, "Show only the classes whose full name contains this text (case ignored)");
        if (ImGui::RadioButton("namespaces", tab.view == ClassView::kNamespaces))
            tab.view = ClassView::kNamespaces;
        StatusHint(_status, "List the classes by namespace");
        ImGui::SameLine();
        if (ImGui::RadioButton("folders", tab.view == ClassView::kFolders))
            tab.view = ClassView::kFolders;
        StatusHint(_status, "List the classes by the folder of their file");

        bool changed = false;
        ImGui::BeginDisabled(tab.classes.empty());
        const std::string clearLabel = tab.classes.empty()
                                           ? std::string("Clear selection###clear")
                                           : "Clear selection (" + std::to_string(tab.classes.size()) + ")###clear";
        if (ImGui::SmallButton(clearLabel.c_str()))
        {
            tab.classes.clear();
            changed = true;
        }
        ImGui::EndDisabled();
        StatusHint(_status, tab.classes.empty() ? "No class checked"
                                                : "Uncheck all the classes, also those hidden by the search");
        ImGui::Separator();

        if (_result.model.classes.empty())
            ImGui::TextDisabled("No classes in the C++ files analyzed.");
        else
            changed |= RenderScope(tab, tab.view == ClassView::kFolders ? _folderTree : _tree, std::string(), 0);

        if (changed)
            GenerateDiagram(tab);
    }

    // width 0: all the width left.
    void CodeGraphPanel::RenderCanvas(DiagramTab& tab, float width)
    {
        // The canvas has no scroll bars: the diagram is drawn at pan from
        // the top left corner, so it can go anywhere, also when smaller than
        // the view, and zooming keeps the point under the mouse in place.
        ImGui::BeginChild("##canvas", ImVec2(width, 0.0f), ImGuiChildFlags_None,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        const ImVec2 origin    = ImGui::GetCursorScreenPos();
        const ImVec2 available = ImGui::GetContentRegionAvail();
        if (tab.classes.empty())
        {
            ImGui::TextDisabled(tab.showFilters ? "Check the classes to draw in the filters at the right."
                                                : "Check the classes to draw: open the Filters.");
            ImGui::EndChild();
            return;
        }
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
    }
}
