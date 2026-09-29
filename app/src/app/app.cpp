#include "app.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>

#include <ImGuiDot.h>
#include <imgui.h>
#include <vulkan/vulkan.h>

#include "version.h"

#include "../platform/sdl_session.h"
#include "../platform/asset_paths.h"
#include "../platform/window.h"
#include "../platform/vulkan_context.h"
#include "../platform/swapchain.h"
#include "../platform/frame_resources.h"
#include "../platform/imgui_backend.h"
#include "../ui/theme.h"
#include "../ui/font_library.h"
#include "../ui/markdown_renderer.h"
#include "../ui/document_panel_host.h"
#include "../ui/project_selector_panel.h"
#include "../ui/shell.h"
#include "../services/document_loader.h"
#include "../services/config_resolver.h"
#include "../services/scaffold_repository.h"
#include "../services/diff_engine.h"
#include "../services/promote_engine.h"
#include "../services/apply_engine.h"
#include "../services/snapshot_service.h"
#include "../core/project.h"

namespace dev_dash::app
{
    App::App() = default;

    App::~App()
    {
        // The shell's diagram panel frees its Graphviz layout on destruction:
        // it must go before the Graphviz context that ImGuiDot::CleanUp() frees.
        _shell.reset();
        if (_diagramsAvailable)
            ImGuiDot::CleanUp();

        // ImGui::DestroyContext() asserts that all backends are already shut
        // down. ImGuiBackend::~ImGuiBackend() calls ImGui_ImplVulkan_Shutdown()
        // + ImGui_ImplSDL3_Shutdown(), but member destructors run AFTER the
        // destructor body — so we must explicitly reset the backend first.
        _imguiBackend.reset();
        if (ImGui::GetCurrentContext())
            ImGui::DestroyContext();
    }

    int App::Run()
    {
        try
        {
            if (!Init())
                return EXIT_FAILURE;
            if (!MainLoop())
                return EXIT_FAILURE;
            return EXIT_SUCCESS;
        }
        catch (const std::exception& e)
        {
            std::cerr << "[error] " << e.what() << '\n';
            return EXIT_FAILURE;
        }
    }

    bool App::Init()
    {
        _sdlSession     = std::make_unique<platform::SdlSession>();
        _window         = std::make_unique<platform::Window>(
            std::string("dev-dash ") + kVersion,
            platform::kInitialWindowWidth, platform::kInitialWindowHeight);
        _vulkanContext  = std::make_unique<platform::VulkanContext>(*_window);
        _swapchain      = std::make_unique<platform::Swapchain>(*_vulkanContext, *_window);
        _frameResources = std::make_unique<platform::FrameResources>(*_vulkanContext, *_swapchain);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // Fonts and style metrics scale with the display content scale
        // (1.0 on classic 96-dpi, >1 on hi-dpi/2K+ screens).
        float uiScale = SDL_GetWindowDisplayScale(_window->Handle());
        if (uiScale <= 0.0f)
            uiScale = 1.0f;

        const std::filesystem::path assetsDir = platform::ResolveAssetsDir();
        _fonts        = std::make_unique<ui::FontLibrary>(uiScale, assetsDir / "fonts");
        _imguiBackend = std::make_unique<platform::ImGuiBackend>(*_window, *_vulkanContext, *_swapchain);

        // After ImGuiBackend: its init sets the stock dark style as a
        // baseline, the theme overrides it with our visual identity.
        ui::ApplyTheme(ui::GrafiteAmbraTheme(), uiScale);

        _documentLoader    = std::make_unique<services::DocumentLoader>();
        _markdownRenderer  = std::make_unique<ui::MarkdownRenderer>(*_fonts);
        _documentPanelHost = std::make_unique<ui::DocumentPanelHost>(*_documentLoader, *_markdownRenderer);

        _configResolver     = std::make_unique<services::ConfigResolver>();
        _diffEngine         = std::make_unique<services::DiffEngine>();
        _promoteEngine      = std::make_unique<services::PromoteEngine>();
        _applyEngine        = std::make_unique<services::ApplyEngine>();
        _snapshotService    = std::make_unique<services::SnapshotService>(*_applyEngine);
        _scaffoldRepository = std::make_unique<services::ScaffoldRepository>();
        EnsureRuntimeDirs();

        // Graphviz context for the diagrams. A failure only disables them.
        _diagramsAvailable = ImGuiDot::Initialize();
        if (!_diagramsAvailable)
            _startupIssues.emplace_back(
                "Graphviz could not be initialized — diagrams are disabled this session.");

        _projectSelectorPanel = std::make_unique<ui::ProjectSelectorPanel>(
            _window->Handle(),
            [this](const std::filesystem::path& path) { OnProjectSelected(path); });

        _window->Show();
        return true;
    }

    void App::EnsureRuntimeDirs()
    {
        const char* home = std::getenv("HOME");
        if (!home)
        {
            _startupIssues.emplace_back(
                "$HOME is not set — scaffolds and snapshots are disabled this session.");
            return;
        }

        const std::filesystem::path devdash   = std::filesystem::path(home) / ".devdash";
        const std::filesystem::path scaffolds = devdash / "scaffolds";
        const std::filesystem::path snapshots = devdash / "snapshots";

        for (const auto& dir : {scaffolds, snapshots})
        {
            std::error_code ec;
            std::filesystem::create_directories(dir, ec);
            if (ec)
            {
                const std::string msg =
                    "Could not create " + dir.string() + ": " + ec.message();
                std::cerr << "[error] " << msg << '\n';
                _startupIssues.push_back(msg);
            }
        }

        // Wire the roots regardless: if a dir failed, the service simply finds
        // it absent later — the issue is already surfaced above, not silenced.
        _scaffoldRepository->SetScaffoldRoot(scaffolds);
        _snapshotService->SetSnapshotRoot(snapshots);
    }

    void App::RenderStartupIssues()
    {
        if (_startupIssues.empty())
            return;

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(520.0f, 0.0f), ImGuiCond_Appearing);

        bool open = true;
        ImGui::Begin("Startup warnings", &open,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextWrapped(
            "DevDash hit problems preparing its working directories. Scaffolds "
            "and snapshots may not work until these are resolved:");
        ImGui::Spacing();
        for (const auto& issue : _startupIssues)
            ImGui::BulletText("%s", issue.c_str());
        ImGui::Spacing();
        if (ImGui::Button("Dismiss"))
            open = false;
        ImGui::End();

        if (!open)
            _startupIssues.clear();
    }

    void App::OnProjectSelected(const std::filesystem::path& path)
    {
        _shell = std::make_unique<ui::Shell>(
            *_configResolver, *_scaffoldRepository, *_diffEngine,
            *_promoteEngine, *_applyEngine, *_snapshotService,
            *_documentPanelHost, core::Project{path}, _diagramsAvailable);
        _appState = AppState::kWorkspace;
    }

    bool App::MainLoop()
    {
        while (true)
        {
            SDL_Event event;
            while (_window->IsPaused()
                       ? SDL_WaitEvent(&event)
                       : _window->PollEvent(event))
            {
                _imguiBackend->ProcessEvent(event);
                _window->ProcessEvent(event);
                if (_window->ShouldClose())
                    goto done;
            }
            if (_window->ShouldClose())
                break;

            _imguiBackend->NewFrame();

            RenderStartupIssues();

            switch (_appState)
            {
            case AppState::kSelectingProject:
                _projectSelectorPanel->Render();
                break;

            case AppState::kWorkspace:
                _shell->Render();
                if (_shell->WantsProjectSwitch())
                {
                    _shell.reset();
                    _appState = AppState::kSelectingProject;
                }
                break;
            }

            // Store fence handle locally: InFlightFence() returns by value and
            // vkWaitForFences / vkResetFences need a pointer.
            VkFence fence = _frameResources->InFlightFence();

            if (vkWaitForFences(
                    _vulkanContext->Device(), 1, &fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
            {
                std::cerr << "[error] vkWaitForFences failed\n";
                return false;
            }

            uint32_t imageIndex = 0;
            const VkResult acquire = vkAcquireNextImageKHR(
                _vulkanContext->Device(),
                _swapchain->Handle(),
                UINT64_MAX,
                _frameResources->ImageAvailableSemaphore(),
                VK_NULL_HANDLE,
                &imageIndex);
            if (acquire == VK_ERROR_OUT_OF_DATE_KHR || acquire == VK_SUBOPTIMAL_KHR)
            {
                // NewFrame() was already called; must end the frame before skipping Render().
                ImGui::EndFrame();
                _swapchain->Recreate(*_vulkanContext, *_window);
                continue;
            }
            else if (acquire != VK_SUCCESS)
            {
                std::cerr << "[error] vkAcquireNextImageKHR failed\n";
                return false;
            }

            if (vkResetFences(_vulkanContext->Device(), 1, &fence) != VK_SUCCESS)
            {
                std::cerr << "[error] vkResetFences failed\n";
                return false;
            }

            VkCommandBuffer cmdBuf = _frameResources->CurrentCommandBuffer();

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            if (vkBeginCommandBuffer(cmdBuf, &beginInfo) != VK_SUCCESS)
            {
                std::cerr << "[error] vkBeginCommandBuffer failed\n";
                return false;
            }

            const ImVec4 bg = ui::CurrentTheme().windowBg;
            const VkClearValue clearColor = {.color = {.float32 = {bg.x, bg.y, bg.z, 1.0f}}};

            VkRenderPassBeginInfo rpBegin = {};
            rpBegin.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rpBegin.renderPass        = _swapchain->RenderPass();
            rpBegin.framebuffer       = _swapchain->Framebuffers()[imageIndex];
            rpBegin.renderArea.offset = {0, 0};
            rpBegin.renderArea.extent = _swapchain->Extent();
            rpBegin.clearValueCount   = 1;
            rpBegin.pClearValues      = &clearColor;
            vkCmdBeginRenderPass(cmdBuf, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

            _imguiBackend->Render(cmdBuf);

            vkCmdEndRenderPass(cmdBuf);

            if (vkEndCommandBuffer(cmdBuf) != VK_SUCCESS)
            {
                std::cerr << "[error] vkEndCommandBuffer failed\n";
                return false;
            }

            const VkSemaphore waitSems[]   = {_frameResources->ImageAvailableSemaphore()};
            const VkSemaphore signalSems[] = {_frameResources->RenderFinishedSemaphoreForImage(imageIndex)};
            constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

            VkSubmitInfo submit = {};
            submit.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit.waitSemaphoreCount   = 1;
            submit.pWaitSemaphores      = waitSems;
            submit.pWaitDstStageMask    = waitStages;
            submit.commandBufferCount   = 1;
            submit.pCommandBuffers      = &cmdBuf;
            submit.signalSemaphoreCount = 1;
            submit.pSignalSemaphores    = signalSems;

            if (vkQueueSubmit(
                    _vulkanContext->GraphicsQueue(), 1, &submit,
                    _frameResources->InFlightFence()) != VK_SUCCESS)
            {
                std::cerr << "[error] vkQueueSubmit failed\n";
                return false;
            }

            const VkSwapchainKHR swapchains[] = {_swapchain->Handle()};
            VkPresentInfoKHR present = {};
            present.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            present.waitSemaphoreCount = 1;
            present.pWaitSemaphores    = signalSems;
            present.swapchainCount     = 1;
            present.pSwapchains        = swapchains;
            present.pImageIndices      = &imageIndex;

            const VkResult presentResult = vkQueuePresentKHR(_vulkanContext->PresentQueue(), &present);
            if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR
                || _window->ConsumeResizeFlag())
            {
                _swapchain->Recreate(*_vulkanContext, *_window);
            }
            else if (presentResult != VK_SUCCESS)
            {
                std::cerr << "[error] vkQueuePresentKHR failed\n";
                return false;
            }

            _frameResources->AdvanceFrame();
        }

    done:
        vkDeviceWaitIdle(_vulkanContext->Device());
        return true;
    }
}
