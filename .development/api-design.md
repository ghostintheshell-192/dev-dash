# API Design — Layered Architecture

**Status**: definitive (final design after review of agent proposals).
**Companion to**: [ADR-010](reference/decisions/010-architecture-design.md) — rationale lives there; this document specifies *what* the post-refactor surface looks like.
**Scope**: all `.h` signatures for the layered tree under `app/src/`. No `.cpp` content. CMake details out of scope (will live in subdirectory `CMakeLists.txt` files at implementation time).

## Filesystem layout

```
app/
├── CMakeLists.txt                   (root: project setup, compiler dispatch, CPM, subdirs)
├── CMakePresets.json                (linux-debug / linux-release)
├── cmake/                           (compiler dispatch + CPM bootstrap)
├── external/                        (CPM deps + ImGui static lib targets — invariato)
├── assets/                          (fonts — invariato)
└── src/
    ├── CMakeLists.txt               (target dev-dash + target_precompile_headers + asset copy)
    ├── main.cpp                     (int main → dev_dash::app::App().Run())
    ├── pch.h                        (force-included via target_precompile_headers)
    ├── core/
    │   ├── CMakeLists.txt
    │   ├── project.h
    │   ├── config_layer.h
    │   ├── effective_config.h
    │   ├── scaffold.h
    │   ├── snapshot.h
    │   └── diff_entry.h
    ├── services/
    │   ├── CMakeLists.txt
    │   ├── document_loader.{h,cpp}
    │   ├── config_resolver.{h,cpp}
    │   ├── diff_engine.{h,cpp}
    │   ├── apply_engine.{h,cpp}
    │   ├── snapshot_service.{h,cpp}
    │   └── scaffold_repository.{h,cpp}
    ├── ui/
    │   ├── CMakeLists.txt
    │   ├── font_library.{h,cpp}
    │   ├── markdown_renderer.{h,cpp}
    │   └── document_panel_host.{h,cpp}
    ├── platform/
    │   ├── CMakeLists.txt
    │   ├── sdl_session.{h,cpp}
    │   ├── window.{h,cpp}
    │   ├── vulkan_context.{h,cpp}
    │   ├── swapchain.{h,cpp}
    │   ├── frame_resources.{h,cpp}
    │   ├── imgui_backend.{h,cpp}
    │   └── deletion_queue.h
    └── app/
        ├── CMakeLists.txt
        └── app.{h,cpp}
```

Conventions (see [coding-standards.md](../.claude/rules/coding-standards.md)):
files `snake_case`, types `PascalCase`, methods `PascalCase`, locals/params/public-struct-fields `camelCase`, private fields `_camelCase`, constants/enum-values `kPascalCase`. 4-space indent, Allman braces, explicit `std::`.

---

## core/

Pure value types. Header-only. Depend only on `<filesystem>`, `<vector>`, `<string>`, `<optional>`. No vtables.

### `Project`

Reference point: which directory are we looking at, and what hints about Claude Code config does it have?

```cpp
namespace dev_dash::core
{
    struct Project
    {
        std::filesystem::path path;
        bool hasClaudeDir = false;    // ./.claude/ exists
        bool hasGit       = false;    // ./.git/ exists
        bool hasClaudeMd  = false;    // ./CLAUDE.md exists

        Project() = default;
        explicit Project(const std::filesystem::path& projectPath);

        // Re-stat the filesystem and refresh presence flags.
        void RefreshPresenceFlags();
    };
}
```

Pure data. Constructor populates flags non-throwingly. Refresh exists for external edits.

### `ConfigLayer`

Labeled pointer to a directory containing Claude rules. Doesn't validate; that's `ConfigResolver`'s job.

```cpp
namespace dev_dash::core
{
    enum class ConfigLayerKind
    {
        kGlobal,    // ~/.claude/
        kWorkspace, // <workspace>/.claude/ (optional, may be empty)
        kProject    // <project>/.claude/
    };

    struct ConfigLayer
    {
        ConfigLayerKind kind;
        std::filesystem::path path;
        std::string displayName;      // "Global", "Workspace", "Project"

        ConfigLayer() = default;
        ConfigLayer(ConfigLayerKind layerKind,
                    std::filesystem::path layerPath,
                    std::string layerDisplayName);
    };
}
```

### `EffectiveConfig`

Result of merging Global / Workspace / Project layers. Class with invariants (every node references a known layer). Immutable after construction.

```cpp
namespace dev_dash::core
{
    struct ConfigNode
    {
        std::string relativePath;             // "rules/overview.md", "CLAUDE.md"
        ConfigLayerKind sourceLayer;          // which layer this node came from
        std::filesystem::path sourceFilePath; // absolute, for UI navigation
    };

    class EffectiveConfig
    {
    public:
        EffectiveConfig() = default;

        // Throws std::invalid_argument if any node has duplicate relativePath.
        explicit EffectiveConfig(std::vector<ConfigNode> nodes);

        const std::vector<ConfigNode>& Nodes() const { return _nodes; }

        // Returns nullptr if not found.
        const ConfigNode* FindNode(std::string_view relativePath) const;

        std::vector<const ConfigNode*> NodesFromLayer(ConfigLayerKind layer) const;

        // std::nullopt if relativePath not in this config.
        std::optional<ConfigLayerKind> SourceLayerOf(std::string_view relativePath) const;

    private:
        std::vector<ConfigNode> _nodes;
    };
}
```

Note: merge policy (deep-merge JSON vs concat markdown vs union hooks) lives in `ConfigResolver`, not here. This type only holds the result.

### `Scaffold`

Reference to a user-managed scaffold under `~/.devdash/scaffolds/<name>/`. Pure data.

```cpp
namespace dev_dash::core
{
    struct Scaffold
    {
        std::string name;                 // "scaffold-coding"
        std::filesystem::path path;       // ~/.devdash/scaffolds/scaffold-coding/
        std::string description;          // optional
        bool isDefault = false;           // marker file or config flag
    };
}
```

### `Snapshot`

Reference to a point-in-time backup under `~/.devdash/snapshots/<projectSlug>/<timestamp>-<slug>/`. Pure data.

```cpp
namespace dev_dash::core
{
    enum class SnapshotKind
    {
        kExplicit, // user-created
        kAuto      // auto-created before destructive op
    };

    struct Snapshot
    {
        SnapshotKind kind;
        std::string projectSlug;          // derived from project path (impl-defined)
        std::string name;                 // "before-major-changes" or "auto-pre-apply-X"
        std::filesystem::path path;
        std::string timestamp;            // ISO 8601
        std::string description;          // optional
        std::string originatingAction;    // for auto: "apply-scaffold-X", "restore-Y"
    };
}
```

### `DiffEntry`

One file-level diff result between two trees. Pure data; payload is optional and lazy.

```cpp
namespace dev_dash::core
{
    enum class DiffKind
    {
        kUnchanged, // file in both, identical content
        kModified,  // file in both, differs
        kMissing,   // file in source only (not in target)
        kCustom     // file in target only (not in source)
    };

    struct DiffEntry
    {
        std::string relativePath;
        DiffKind kind;
        std::optional<std::string> sourceContent; // populated iff caller asked + kind == kModified
        std::optional<std::string> targetContent; // populated iff caller asked + kind == kModified
    };
}
```

---

## services/

Concrete classes (no virtual destructors, no abstract bases — see ADR-010 §3).
Constructor-injected by reference. Test fixtures use `std::filesystem::temp_directory_path()`.

### `DocumentLoader`

Refactor of `Renderer::PreprocessImports`. Reads markdown + rewrites `@`-imports as `claudeimport://` links, fence-aware.

```cpp
namespace dev_dash::services
{
    struct LoadResult
    {
        std::string content;                            // processed markdown
        std::vector<std::filesystem::path> imports;     // resolved absolute paths
    };

    class DocumentLoader
    {
    public:
        DocumentLoader() = default;

        // Returns empty content + empty imports on read failure (logs warning).
        // Fence-aware: @-imports inside fenced code blocks are not rewritten.
        LoadResult Load(const std::filesystem::path& filePath);
    };
}
```

### `ConfigResolver`

Merges Global / Workspace (optional) / Project layers into an `EffectiveConfig`. Pure combiner; merge semantics evolve here without changing the API.

```cpp
namespace dev_dash::core { class EffectiveConfig; struct Project; struct ConfigLayer; }

namespace dev_dash::services
{
    class ConfigResolver
    {
    public:
        ConfigResolver() = default;

        // global is required; project is required; workspace optional (kind = kWorkspace,
        // empty path = treated as absent). Throws std::runtime_error if global is unreadable.
        // Non-throwing on missing project layer (treated as "inherit from global").
        core::EffectiveConfig Resolve(
            const core::Project& project,
            const core::ConfigLayer& global,
            const core::ConfigLayer& projectLayer,
            const core::ConfigLayer& workspace = core::ConfigLayer{});

        // Convenience: derive global as ~/.claude/ via $HOME.
        core::EffectiveConfig ResolveWithDefaultGlobal(
            const core::Project& project,
            const core::ConfigLayer& projectLayer);
    };
}
```

Note: today's behavior is "later layer overrides earlier, file-by-file". Granularità più fine (sezione markdown, deep-merge JSON) è una evoluzione interna che non cambia la firma.

### `DiffEngine`

File-by-file comparison of two filesystem trees. Output is a sorted vector of `DiffEntry`. Content payload is opt-in.

```cpp
namespace dev_dash::core { struct DiffEntry; }

namespace dev_dash::services
{
    class DiffEngine
    {
    public:
        DiffEngine() = default;

        // Recursively walks both trees. Missing trees treated as empty (non-throwing).
        // If includeContent = true, populates sourceContent/targetContent for kModified entries.
        // Result is ordered by relativePath.
        std::vector<core::DiffEntry> CompareTrees(
            const std::filesystem::path& sourceRoot,
            const std::filesystem::path& targetRoot,
            bool includeContent = false);
    };
}
```

### `ApplyEngine`

Transactional write of selected files from a source tree (scaffold or snapshot) into a target project. Optional `SnapshotService*` for explicit autosnapshot trigger.

```cpp
namespace dev_dash::services
{
    class SnapshotService;

    class ApplyEngine
    {
    public:
        struct ApplyConfig
        {
            std::vector<std::string> filesToApply; // relative paths from source
            bool createAutosnapshot = true;
            bool forceOverwrite = false;           // false = skip-if-exists; true = overwrite
        };

        ApplyEngine() = default;

        // Transactional: stages writes, atomic-moves on success, rolls back on failure.
        // If config.createAutosnapshot is true, snapshotService must be non-null
        // (throws std::invalid_argument otherwise).
        // Returns true on success; false if any write failed (target unchanged, autosnapshot
        // already created if requested).
        bool Apply(
            const std::filesystem::path& sourceRoot,
            const std::filesystem::path& targetRoot,
            const ApplyConfig& config,
            SnapshotService* snapshotService = nullptr);
    };
}
```

Note: pointer (not reference) per `SnapshotService` perché è veramente opzionale.
Convertibile a `std::optional<std::reference_wrapper<SnapshotService>>` se gli `*` ti danno fastidio, ma raw pointer è idiomatico per "optional non-owning dependency" in C++20.

### `SnapshotService`

Save / list / restore / prune snapshots. Restore = apply with `forceOverwrite = true` (default policy).

```cpp
namespace dev_dash::core { struct Project; struct Snapshot; }

namespace dev_dash::services
{
    class ApplyEngine;

    class SnapshotService
    {
    public:
        // ApplyEngine& injected because Restore is "apply with forced overwrite".
        explicit SnapshotService(ApplyEngine& applyEngine);

        // Explicit user snapshot.
        core::Snapshot SaveExplicit(
            const core::Project& project,
            std::string_view name,
            std::string_view description = {});

        // Auto snapshot before a destructive operation. Marker "auto-pre-<action>-...".
        core::Snapshot SaveAuto(
            const core::Project& project,
            std::string_view action);

        // Reverse-chronological order. Returns empty vector if project has no snapshots.
        std::vector<core::Snapshot> List(std::string_view projectSlug);

        // Restore snapshot to its origin project. Creates "auto-pre-restore-..." snapshot first.
        // Throws std::invalid_argument if snapshot.projectSlug doesn't match target.
        bool Restore(
            const core::Snapshot& snapshot,
            const core::Project& target);

        // Prune autosnapshots beyond max count. Explicit snapshots are never pruned.
        // Returns number deleted.
        int PruneAuto(std::string_view projectSlug, int maxAutoSnapshots = 10);

        std::filesystem::path SnapshotRoot() const { return _snapshotRoot; }
        void SetSnapshotRoot(std::filesystem::path root);

    private:
        ApplyEngine& _applyEngine;
        std::filesystem::path _snapshotRoot;       // default: ~/.devdash/snapshots/
    };
}
```

Note: `projectSlug` derivation è interna; oggi probabilmente basename + hash suffix. Open question in spec snapshot-history.md, lasciata aperta in ADR-010.

### `ScaffoldRepository`

Discover + list scaffolds in `~/.devdash/scaffolds/`. Discovery cached; explicit `Refresh()` re-scans.

```cpp
namespace dev_dash::core { struct Scaffold; }

namespace dev_dash::services
{
    class ScaffoldRepository
    {
    public:
        ScaffoldRepository() = default;

        // Lazy: scans on first call, caches result. Use Refresh() to re-scan.
        std::vector<core::Scaffold> List();

        // Returns nullptr if not found. Pointer invalidated by next Refresh()/List().
        const core::Scaffold* Find(std::string_view name);

        // Marked default (alphabetically first if multiple). nullptr if no scaffolds.
        const core::Scaffold* GetDefault();

        void Refresh();

        std::filesystem::path ScaffoldRoot() const { return _scaffoldRoot; }
        void SetScaffoldRoot(std::filesystem::path root);

    private:
        std::filesystem::path _scaffoldRoot;       // default: ~/.devdash/scaffolds/
        std::vector<core::Scaffold> _cache;
        bool _cached = false;
    };
}
```

### `PromoteEngine`

The reverse of `ApplyEngine`: copy selected files from a project *back* into a
scaffold, so a refined project config becomes a reusable template. Returns a
count plus a list of per-file errors (no rollback — promotion targets a
user-owned scaffold dir, not a live project).

```cpp
namespace dev_dash::services
{
    class PromoteEngine
    {
    public:
        struct Result
        {
            int                      copiedCount = 0;
            std::vector<std::string> errors;

            bool Ok() const { return errors.empty(); }
        };

        // Copy each relative path from projectRoot to scaffoldRoot,
        // creating intermediate directories as needed.
        Result Promote(
            const std::filesystem::path&    projectRoot,
            const std::filesystem::path&    scaffoldRoot,
            const std::vector<std::string>& relativePaths) const;
    };
}
```

### Config adapters (section-aware)

> **Evolution note.** `EffectiveConfig` grew past the flat node list shown in
> the `core/` section above: the resolved config is now a list of
> **`ConfigSection`** (one per category), each carrying whether it is always
> in Claude's context or loaded on demand. The adapters below produce those
> sections; `ConfigResolver` orchestrates them.

```cpp
namespace dev_dash::core
{
    enum class ConfigSectionKind
    {
        kClaudeMd, kRules, kMemory, kSkills, kAgents, kMcpServers, kHooks,
    };

    struct ConfigSection
    {
        ConfigSectionKind       kind;
        std::string             label;
        std::vector<ConfigNode> nodes;
        bool                    alwaysInContext;  // false → on-demand, 0 tokens until triggered
    };
}
```

Two shared building blocks the adapters sit on:

```cpp
namespace dev_dash::services
{
    // Directory scanning + @-include resolution.
    class ConfigFileScanner
    {
    public:
        // Files in `dir` matching `predicate`; recursive optional.
        std::vector<std::filesystem::path> Scan(
            const std::filesystem::path& dir,
            const std::function<bool(const std::filesystem::path&)>& predicate,
            bool recursive = false) const;

        // Follow @path directives in a markdown file, transitively (maxDepth
        // hops), resolved relative to each including file's directory.
        std::vector<std::filesystem::path> ResolveAtIncludes(
            const std::filesystem::path& file,
            int maxDepth = 5) const;
    };

    // Pull MCP servers / hooks out of a settings.json.
    class SettingsParser
    {
    public:
        std::vector<core::ConfigNode> ParseMcpServers(
            const std::filesystem::path& settingsPath,
            core::ConfigLayerKind layer) const;

        std::vector<core::ConfigNode> ParseHooks(
            const std::filesystem::path& settingsPath,
            core::ConfigLayerKind layer) const;
    };
}
```

Each adapter resolves one section across the layers it cares about, returning
a `core::ConfigSection`. They take a `ConfigFileScanner&` by reference
(constructor injection); the `kMcpServers`/`kHooks` ones use `SettingsParser`.

| Adapter | Section | Layers consulted |
|---------|---------|------------------|
| `ClaudeMdAdapter` | `kClaudeMd` | global, workspace, project |
| `RulesAdapter` | `kRules` | global, project |
| `MemoryAdapter` | `kMemory` | (per memory layout) |
| `SkillsAdapter` | `kSkills` | (on-demand section) |
| `AgentsAdapter` | `kAgents` | (on-demand section) |
| `McpAdapter` | `kMcpServers` | via `SettingsParser` |
| `HooksAdapter` | `kHooks` | via `SettingsParser` |

```cpp
namespace dev_dash::services
{
    class ClaudeMdAdapter
    {
    public:
        explicit ClaudeMdAdapter(ConfigFileScanner& scanner);

        core::ConfigSection Resolve(const core::ConfigLayer& global,
                                    const core::ConfigLayer& workspace,
                                    const core::ConfigLayer& project) const;
    private:
        ConfigFileScanner& _scanner;
    };
    // RulesAdapter, MemoryAdapter, SkillsAdapter, AgentsAdapter,
    // McpAdapter, HooksAdapter follow the same shape (Resolve(...) ->
    // core::ConfigSection), differing only in which layers they read.
}
```

`adapter_utils.h` provides the shared `AppendNodes()` helper (dedupe by
`sourceFilePath`, label relative to a base dir) and the `IsMd()` predicate.

---

## ui/

### `FontLibrary`

App-wide font resource. Loads IBM Plex Sans family + DejaVu fallback into ImGui's atlas
exactly once. Owned by `App`, referenced (const) by every UI panel that picks fonts.

```cpp
namespace dev_dash::ui
{
    class FontLibrary
    {
    public:
        // Construct AFTER ImGui::CreateContext() has been called and BEFORE
        // ImGui_ImplVulkan_Init() (which builds the font texture).
        FontLibrary();
        ~FontLibrary() = default;

        FontLibrary(const FontLibrary&)            = delete;
        FontLibrary& operator=(const FontLibrary&) = delete;

        ImFont* Regular() const { return _regular; }
        ImFont* Italic()  const { return _italic; }
        ImFont* Bold()    const { return _bold; }
        ImFont* BoldH1()  const { return _boldH1; }
        ImFont* BoldH2()  const { return _boldH2; }
        ImFont* BoldH3()  const { return _boldH3; }

    private:
        ImFont* _regular = nullptr;
        ImFont* _italic  = nullptr;
        ImFont* _bold    = nullptr;
        ImFont* _boldH1  = nullptr;
        ImFont* _boldH2  = nullptr;
        ImFont* _boldH3  = nullptr;
    };
}
```

Note: ImGui possiede l'atlas (e quindi i puntatori `ImFont*`); `FontLibrary` non li
delete-a. Distruzione di `FontLibrary` lascia l'atlas intatto fino allo shutdown di ImGui.

### `MarkdownRenderer`

Pure markdown renderer. Deriva da `imgui_md`. Non sa nulla di panel o queue: un
`LinkHandler` callback decide cosa fare per ogni click.

```cpp
namespace dev_dash::ui
{
    class FontLibrary;

    class MarkdownRenderer : public imgui_md
    {
    public:
        using LinkHandler = std::function<void(std::string_view url)>;

        explicit MarkdownRenderer(const FontLibrary& fonts);
        ~MarkdownRenderer() = default;

        MarkdownRenderer(const MarkdownRenderer&)            = delete;
        MarkdownRenderer& operator=(const MarkdownRenderer&) = delete;

        // Default handler opens external URLs via SDL_OpenURL.
        // DocumentPanelHost overrides this to intercept claudeimport:// links.
        void SetLinkHandler(LinkHandler handler);

    protected:
        ImFont* get_font() const override;
        void open_url() const override;          // calls _linkHandler(m_href)
        bool get_image(image_info&) const override { return false; }
        void SPAN_CODE(bool e) override;
        void BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool e) override;

    private:
        const FontLibrary& _fonts;
        LinkHandler _linkHandler;                // default: SDL_OpenURL
    };
}
```

### `DocumentPanelHost`

Possiede la lista di markdown panel aperti; gestisce open/load/render/close + intercetta
i `claudeimport://` link iscrivendosi come `LinkHandler` su `MarkdownRenderer`.

```cpp
namespace dev_dash::services { class DocumentLoader; }

namespace dev_dash::ui
{
    class MarkdownRenderer;

    class DocumentPanelHost
    {
    public:
        DocumentPanelHost(services::DocumentLoader& loader,
                          MarkdownRenderer& renderer);
        ~DocumentPanelHost() = default;

        DocumentPanelHost(const DocumentPanelHost&)            = delete;
        DocumentPanelHost& operator=(const DocumentPanelHost&) = delete;

        // Idempotent: opening an already-open path is a no-op.
        void OpenPanel(const std::filesystem::path& path);

        // Called once per frame from App::Run() between ImGui::NewFrame() and ImGui::Render().
        void Render();

    private:
        struct Panel
        {
            std::string title;     // e.g. "CLAUDE.md##/abs/path/CLAUDE.md"
            std::filesystem::path path;
            std::string content;
            bool open = true;
        };

        // Invoked by MarkdownRenderer when a claudeimport:// link is clicked.
        // Pushes the path into _pendingImports for the next Render() pass.
        void HandleLinkClick(std::string_view url);

        void DrainPendingImports();

        services::DocumentLoader& _loader;
        MarkdownRenderer& _renderer;
        std::vector<Panel> _panels;
        std::vector<std::filesystem::path> _pendingImports;
    };
}
```

Note: `_pendingImports` è interno a `DocumentPanelHost`. `MarkdownRenderer` ci arriva
indirettamente via callback (`HandleLinkClick` lo popola). Sull'ordine di distruzione,
vedi sezione **Cross-cutting** sotto.

---

## platform/

### `SdlSession`

RAII per `SDL_Init` / `SDL_Quit`. Owned by `App`, costruito per primo, distrutto per ultimo.

```cpp
namespace dev_dash::platform
{
    class SdlSession
    {
    public:
        // Calls SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS). Throws std::runtime_error on failure.
        SdlSession();
        ~SdlSession();

        SdlSession(const SdlSession&)            = delete;
        SdlSession& operator=(const SdlSession&) = delete;
    };
}
```

### `Window`

Wrappa `SDL_Window*`. Polling è esposto come primitive; resize-flag è esplicito (set da
`ProcessEvent`, consumato da `ConsumeResizeFlag`).

```cpp
namespace dev_dash::platform
{
    inline constexpr int kInitialWindowWidth  = 1280;
    inline constexpr int kInitialWindowHeight = 800;

    class Window
    {
    public:
        Window(std::string_view title, int width, int height);
        ~Window();

        Window(const Window&)            = delete;
        Window& operator=(const Window&) = delete;

        SDL_Window* Handle() const { return _window; }

        // Primitives. Caller polls in a loop and feeds events back via ProcessEvent().
        bool PollEvent(SDL_Event& outEvent) const;

        // Updates _shouldClose / _resized / _paused based on event type.
        // Returns the same event pointer for caller chaining (ImGui_ImplSDL3_ProcessEvent).
        void ProcessEvent(const SDL_Event& event);

        bool ShouldClose() const { return _shouldClose; }
        bool IsPaused()    const { return _paused; }

        // Returns true and clears flag iff the window was resized since the last call.
        bool ConsumeResizeFlag();

        // Drawable size in pixels (post-DPI). May differ from logical window size.
        void GetDrawableSize(int& outWidth, int& outHeight) const;

        void Show();

    private:
        SDL_Window* _window     = nullptr;
        bool _shouldClose       = false;
        bool _resized           = false;
        bool _paused            = false;
    };
}
```

### `VulkanContext`

Vulkan instance + surface + physical device + logical device + queues. Costruisce con un
`Window&` (gli serve la surface).

```cpp
namespace dev_dash::platform
{
    class Window;

    inline constexpr bool kEnableVulkanDebug = true;

    class VulkanContext
    {
    public:
        explicit VulkanContext(const Window& window);
        ~VulkanContext();

        VulkanContext(const VulkanContext&)            = delete;
        VulkanContext& operator=(const VulkanContext&) = delete;

        const vkb::Instance&       Instance()       const { return _instance; }
        const vkb::PhysicalDevice& PhysicalDevice() const { return _physicalDevice; }
        const vkb::Device&         DeviceWrapper()  const { return _device; }

        VkDevice     Device()                   const { return _device.device; }
        VkSurfaceKHR Surface()                  const { return _surface; }
        uint32_t     GraphicsQueueFamilyIndex() const { return _graphicsQueueFamilyIndex; }
        VkQueue      GraphicsQueue()            const { return _graphicsQueue; }
        VkQueue      PresentQueue()             const { return _presentQueue; }

    private:
        vkb::Instance         _instance;
        vkb::PhysicalDevice   _physicalDevice;
        vkb::Device           _device;
        VkSurfaceKHR          _surface = VK_NULL_HANDLE;
        uint32_t              _graphicsQueueFamilyIndex = 0;
        VkQueue               _graphicsQueue = VK_NULL_HANDLE;
        VkQueue               _presentQueue  = VK_NULL_HANDLE;
        DeletionQueue         _deletionQueue;
    };
}
```

Note: ogni classe platform tiene la propria `DeletionQueue` privata, popolata nel
costruttore. RAII ordering è dato dall'ordine di `_deletionQueue.Add()` calls; cleanup LIFO
nel destruttore via `~DeletionQueue()`.

### `Swapchain`

Swapchain + image views + render pass + framebuffers. `Recreate()` per resize.

```cpp
namespace dev_dash::platform
{
    class VulkanContext;
    class Window;

    class Swapchain
    {
    public:
        Swapchain(const VulkanContext& context, const Window& window);
        ~Swapchain();

        Swapchain(const Swapchain&)            = delete;
        Swapchain& operator=(const Swapchain&) = delete;

        VkFormat   ImageFormat() const { return _swapchain.image_format; }
        VkExtent2D Extent()      const { return _swapchain.extent; }
        uint32_t   ImageCount()  const { return _swapchain.image_count; }
        VkSwapchainKHR Handle()  const { return _swapchain.swapchain; }

        VkRenderPass RenderPass() const { return _renderPass; }
        const std::vector<VkFramebuffer>& Framebuffers() const { return _framebuffers; }

        // Wait device idle, tear down dependent objects, rebuild from current window size.
        void Recreate(const VulkanContext& context, const Window& window);

    private:
        vkb::Swapchain             _swapchain;
        std::vector<VkImageView>   _imageViews;
        VkRenderPass               _renderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> _framebuffers;
        VkDevice                   _device = VK_NULL_HANDLE;  // cached for cleanup
    };
}
```

### `FrameResources`

Command pool + command buffers + sync primitives. Owns frame indexing
(`kMaxFramesInFlight` rotation). Non dipende dallo swapchain handle (solo dal suo
`ImageCount()` per i semafori per-image), quindi non ha bisogno di `Recreate()` su resize.

```cpp
namespace dev_dash::platform
{
    class VulkanContext;
    class Swapchain;

    inline constexpr std::size_t kMaxFramesInFlight = 3;

    class FrameResources
    {
    public:
        FrameResources(const VulkanContext& context, const Swapchain& swapchain);
        ~FrameResources();

        FrameResources(const FrameResources&)            = delete;
        FrameResources& operator=(const FrameResources&) = delete;

        std::size_t CurrentFrameIndex() const { return _currentFrame; }

        VkCommandBuffer CurrentCommandBuffer()    const { return _commandBuffers[_currentFrame]; }
        VkSemaphore     ImageAvailableSemaphore() const { return _imageAvailableSemaphores[_currentFrame]; }
        VkFence         InFlightFence()           const { return _inFlightFences[_currentFrame]; }

        // One render-finished semaphore per swapchain image (not per frame in flight).
        VkSemaphore RenderFinishedSemaphoreForImage(uint32_t imageIndex) const;

        // Advance to next frame slot (called after vkQueuePresent).
        void AdvanceFrame();

    private:
        VkDevice      _device      = VK_NULL_HANDLE;
        VkCommandPool _commandPool = VK_NULL_HANDLE;
        std::array<VkCommandBuffer, kMaxFramesInFlight> _commandBuffers{};
        std::array<VkSemaphore, kMaxFramesInFlight>     _imageAvailableSemaphores{};
        std::array<VkFence, kMaxFramesInFlight>         _inFlightFences{};
        std::vector<VkSemaphore> _renderFinishedSemaphoresPerImage;
        std::size_t _currentFrame = 0;
        DeletionQueue _deletionQueue;
    };
}
```

### `ImGuiBackend`

Inizializza ImGui SDL3 + Vulkan backends. **Non** crea il context ImGui (lo fa `App` per
poter inserire la `FontLibrary` fra context-creation e backend-init). **Non** sa di
`ui::`: layer boundary rispettato.

```cpp
namespace dev_dash::platform
{
    class Window;
    class VulkanContext;
    class Swapchain;

    class ImGuiBackend
    {
    public:
        // Pre-condition: ImGui::CreateContext() has been called AND fonts have been
        // added to ImGui::GetIO().Fonts atlas. This constructor calls
        // ImGui_ImplSDL3_InitForVulkan() and ImGui_ImplVulkan_Init() (which builds
        // the font texture from whatever was added).
        ImGuiBackend(const Window& window,
                     const VulkanContext& context,
                     const Swapchain& swapchain);
        ~ImGuiBackend();

        ImGuiBackend(const ImGuiBackend&)            = delete;
        ImGuiBackend& operator=(const ImGuiBackend&) = delete;

        void NewFrame();                       // ImGui_ImplVulkan_NewFrame + ImGui_ImplSDL3_NewFrame + ImGui::NewFrame
        void Render(VkCommandBuffer cmdBuffer); // ImGui::Render + ImGui_ImplVulkan_RenderDrawData

        static void CheckVkResultFn(VkResult err);

    private:
        // No ImGuiContext* member: context is owned by App.
    };
}
```

### `DeletionQueue`

Invariata dal PoC, namespace cambiato. Pattern RAII: stack di lambda invocate in LIFO al
distruttore.

```cpp
// Adapted from Germen Pulchrum (DPD85/Germen, MIT) — `CodaCancellazione`.
// See app/THIRD_PARTY_NOTICES.md for attribution.

namespace dev_dash::platform
{
    class DeletionQueue
    {
    public:
        using Deleter = std::function<void()>;

        DeletionQueue() = default;
        ~DeletionQueue() { Flush(); }

        DeletionQueue(const DeletionQueue&)            = delete;
        DeletionQueue& operator=(const DeletionQueue&) = delete;

        void Add(Deleter d) { _stack.push(std::move(d)); }

        void Flush()
        {
            while (!_stack.empty())
            {
                _stack.top()();
                _stack.pop();
            }
        }

    private:
        std::stack<Deleter> _stack;
    };
}
```

---

## app/

### `App`

Composition root. Possiede ogni layer via `std::unique_ptr`. Costruzione esplicita in
`Init()` body (ordine sequenziale); distruzione automatica LIFO via order of declaration.

**Declaration order (importante)**: `_documentPanelHost` dichiarato PRIMA di
`_markdownRenderer`. Distruzione LIFO inverte: renderer muore prima → `LinkHandler` con
`[this]→host` non viene più invocato → host muore safe dopo. Vedi sezione **Cross-cutting**.

```cpp
namespace dev_dash::app
{
    class App
    {
    public:
        App();
        ~App();

        App(const App&)            = delete;
        App& operator=(const App&) = delete;

        // Returns EXIT_SUCCESS or EXIT_FAILURE.
        int Run();

    private:
        bool Init();
        bool MainLoop();

        // ----- Construction order (in Init body) -----
        // 1. _sdlSession
        // 2. _window
        // 3. _vulkanContext (needs window)
        // 4. _swapchain (needs context + window)
        // 5. _frameResources (needs context + swapchain)
        // 6. ImGui::CreateContext()  -- raw ImGui call, paired with DestroyContext in dtor
        // 7. _fonts                  -- populates atlas
        // 8. _imguiBackend (needs window + context + swapchain; builds font texture)
        // 9. _documentLoader
        // 10. _markdownRenderer (needs fonts)
        // 11. _documentPanelHost (needs loader + renderer; registers LinkHandler)
        //
        // ----- Declaration order (drives destruction) -----
        // Reverse the dependency direction so that callbacks-into-X are torn down
        // before X itself.

        std::unique_ptr<platform::SdlSession>      _sdlSession;
        std::unique_ptr<platform::Window>          _window;
        std::unique_ptr<platform::VulkanContext>   _vulkanContext;
        std::unique_ptr<platform::Swapchain>       _swapchain;
        std::unique_ptr<platform::FrameResources>  _frameResources;
        std::unique_ptr<platform::ImGuiBackend>    _imguiBackend;
        std::unique_ptr<ui::FontLibrary>           _fonts;
        std::unique_ptr<services::DocumentLoader>  _documentLoader;

        // CRITICAL: _documentPanelHost MUST be declared before _markdownRenderer.
        // The host's constructor registers a LinkHandler lambda capturing [this] on
        // the renderer. C++ destroys members in REVERSE declaration order, so
        // _markdownRenderer is destroyed first → its LinkHandler (now never invoked)
        // is safe → _documentPanelHost destroyed after, no dangling capture.
        std::unique_ptr<ui::DocumentPanelHost>     _documentPanelHost;
        std::unique_ptr<ui::MarkdownRenderer>      _markdownRenderer;
    };
}
```

### `main.cpp`

```cpp
#include "app/app.h"

int main(int /*argc*/, char** /*argv*/)
{
    dev_dash::app::App app;
    return app.Run();
}
```

### `pch.h`

```cpp
// app/src/pch.h
#pragma once

// Standard library — stable, used everywhere
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// Heavy third-party — pinned versions, never change between builds
#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <imgui.h>

// NOT in PCH (only included where used):
//   - imgui_impl_sdl3.h, imgui_impl_vulkan.h  (only platform/imgui_backend.cpp)
//   - imgui_md.h                              (only ui/markdown_renderer.{h,cpp})
//   - SDL3/SDL_vulkan.h                       (only platform/window.cpp + vulkan_context.cpp)
//   - <fstream>                               (only services/document_loader.cpp)
//   - Application headers                     (rebuild on change → keep out)
```

CMake wiring (in `app/src/CMakeLists.txt`):

```cmake
target_precompile_headers(dev-dash PRIVATE pch.h)
```

---

## Cross-cutting

### Construction graph

```
                 ┌───────────────┐
                 │  SdlSession   │   (RAII for SDL_Init/SDL_Quit)
                 └───────┬───────┘
                         │
                 ┌───────▼───────┐
                 │    Window     │   (SDL_Window*)
                 └───────┬───────┘
                         │
                 ┌───────▼─────────┐
                 │  VulkanContext  │   (instance/surface/device/queues)
                 └───────┬─────────┘
                         │
              ┌──────────┼──────────┐
              │                     │
       ┌──────▼────────┐    ┌───────▼─────────┐
       │   Swapchain   │    │   (later: FrameResources also)│
       └──────┬────────┘    └─────────────────┘
              │                     │
              └──────────┬──────────┘
                         │
                 ┌───────▼─────────┐
                 │ FrameResources  │
                 └───────┬─────────┘
                         │
                  [ImGui::CreateContext()]   ← raw ImGui call from App
                         │
                 ┌───────▼─────────┐
                 │  FontLibrary    │   (populates atlas)
                 └───────┬─────────┘
                         │
                 ┌───────▼─────────┐
                 │  ImGuiBackend   │   (Init backends, builds font texture)
                 └─────────────────┘

                 ┌─────────────────┐
                 │  DocumentLoader │   (no platform deps)
                 └───────┬─────────┘
                         │
                 ┌───────▼─────────────┐
                 │  MarkdownRenderer   │   (needs FontLibrary)
                 └───────┬─────────────┘
                         │
                 ┌───────▼──────────────┐
                 │  DocumentPanelHost   │   (needs loader + renderer;
                 └──────────────────────┘    registers LinkHandler on renderer)
```

### Destruction order (lifetime trick)

C++ distrugge i membri di una classe nell'ordine inverso di **dichiarazione** (non di
costruzione). Il pattern callback (`MarkdownRenderer::SetLinkHandler` con lambda
`[this]→host`) richiede che il renderer muoia *prima* dell'host.

Soluzione: declarations in `App` mettono `_documentPanelHost` PRIMA di
`_markdownRenderer` nel block privato.

| Ordine costruzione (in `Init()` body) | Ordine distruzione (auto, LIFO declaration) |
| -------------------------------------- | -------------------------------------------- |
| 1. SdlSession                          | 1. MarkdownRenderer  ← muore per primo       |
| 2. Window                              | 2. DocumentPanelHost                         |
| 3. VulkanContext                       | 3. DocumentLoader                            |
| 4. Swapchain                           | 4. FontLibrary                               |
| 5. FrameResources                      | 5. ImGuiBackend                              |
| 6. ImGui::CreateContext()              | 6. FrameResources                            |
| 7. FontLibrary                         | 7. Swapchain                                 |
| 8. ImGuiBackend                        | 8. VulkanContext                             |
| 9. DocumentLoader                      | 9. Window                                    |
| 10. MarkdownRenderer                   | 10. SdlSession  ← muore per ultimo           |
| 11. DocumentPanelHost (registers cb)   | + ImGui::DestroyContext() in `App::~App()`   |

Costruzione vs distruzione divergono di proposito su `_markdownRenderer` ↔
`_documentPanelHost`. Documentato con commento sopra i due membri.

### Layer rules

- `core/` ← nessuno: solo `<filesystem>`, `<vector>`, `<string>`, `<optional>`.
- `services/` ← `core/`.
- `ui/` ← `core/`, `services/`, `<imgui.h>`, `<imgui_md.h>`.
- `platform/` ← `<SDL3>`, `<vulkan.h>`, `<vk-bootstrap>`, `<imgui.h>`. **NON** include `ui/` né `services/`.
- `app/` ← tutti i layer (è la composition root).

Verifica meccanica: nessun `#include "ui/..."` in `platform/*.h`. Nessun `#include "platform/..."` in `services/*.h` né in `core/*.h`. Test mentale: se domani sostituisci ImGui con un altro backend, `core/` e `services/` non dovrebbero richiedere modifiche.

### FontLibrary orchestration

ImGui richiede questo ordine specifico:

```
ImGui::CreateContext()
  ↓
ImGui::GetIO().Fonts->AddFontFromFileTTF(...)        ← popolazione atlas
  ↓
ImGui_ImplVulkan_Init(...)                            ← builda la font texture dall'atlas
```

Quindi `FontLibrary` (che fa `AddFontFromFileTTF`) deve esistere fra `CreateContext()` e
`ImGuiBackend(...)`. In `App::Init()`:

```cpp
ImGui::CreateContext();
_fonts = std::make_unique<ui::FontLibrary>();         // adds fonts to atlas
_imguiBackend = std::make_unique<platform::ImGuiBackend>(*_window, *_vulkanContext, *_swapchain);
// imguiBackend ctor calls ImGui_ImplVulkan_Init, which builds the font texture
```

`ImGui::DestroyContext()` viene chiamato esplicitamente in `App::~App()` dopo che tutti i
membri sono stati distrutti.

### Construction failure handling

`platform::*` constructor che possono fallire (Vulkan device select, swapchain creation):
**throw** `std::runtime_error` con messaggio descrittivo. `App::Init()` cattura,
logga, ritorna `false`. `App::Run()` ritorna `EXIT_FAILURE` su `Init()` falso.

Convenzione: distinto da costruttori "logical errors" (es. `EffectiveConfig` con duplicate paths che lancia `std::invalid_argument`).

---

## Open questions deferred

Tutte queste sono già marcate come open question nelle spec wedge. ADR-010 le lascia
aperte; nessuna è bloccante per il refactor strutturale.

1. **Merge semantics di `ConfigResolver`**: deep-merge JSON, concat markdown, union hooks?
   Spec: `feature-effective-config-view.md`. Oggi: "later layer wins, file-by-file".
2. **Project-slug derivation**: hash, basename, metadata in-project? Spec:
   `feature-snapshot-history.md`.
3. **File watcher / auto-refresh**: v1 manuale, v2 con `inotify`/equivalent. Spec:
   `feature-effective-config-view.md`.
4. **Granularità del diff**: file intero (MVP) vs sezione markdown. Spec:
   `feature-scaffold-management.md` e `feature-effective-config-view.md`.
5. **Conflict resolution su apply**: skip-with-confirmation (default), force, three-way
   merge. Spec: `feature-scaffold-management.md`.
6. **Encoding non-UTF8**: oggi assumiamo UTF-8. Strategia per file legacy: fallback,
   reject, embed-as-binary?
7. **Variabili / placeholder negli scaffold** (`{PROJECT_NAME}`, ecc.): inclinazione
   spec è eliminarli. Da confermare.

---

## Riferimenti

- ADR-010: [reference/decisions/010-architecture-design.md](reference/decisions/010-architecture-design.md)
- Spec wedge: `specs/planned/feature-{effective-config-view,scaffold-management,snapshot-history}.md`
- Coding standards: [.claude/rules/coding-standards.md](../.claude/rules/coding-standards.md), sezione **C++ Conventions**

