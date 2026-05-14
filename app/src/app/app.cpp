#include "app.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>

#include <imgui.h>
#include <vulkan/vulkan.h>

#include "../platform/sdl_session.h"
#include "../platform/window.h"
#include "../platform/vulkan_context.h"
#include "../platform/swapchain.h"
#include "../platform/frame_resources.h"
#include "../platform/imgui_backend.h"
#include "../ui/font_library.h"
#include "../ui/markdown_renderer.h"
#include "../ui/document_panel_host.h"
#include "../ui/project_selector_panel.h"
#include "../ui/effective_config_panel.h"
#include "../ui/scaffold_diff_panel.h"
#include "../ui/snapshot_history_panel.h"
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
            "dev-dash", platform::kInitialWindowWidth, platform::kInitialWindowHeight);
        _vulkanContext  = std::make_unique<platform::VulkanContext>(*_window);
        _swapchain      = std::make_unique<platform::Swapchain>(*_vulkanContext, *_window);
        _frameResources = std::make_unique<platform::FrameResources>(*_vulkanContext, *_swapchain);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        _fonts        = std::make_unique<ui::FontLibrary>();
        _imguiBackend = std::make_unique<platform::ImGuiBackend>(*_window, *_vulkanContext, *_swapchain);

        _documentLoader    = std::make_unique<services::DocumentLoader>();
        _markdownRenderer  = std::make_unique<ui::MarkdownRenderer>(*_fonts);
        _documentPanelHost = std::make_unique<ui::DocumentPanelHost>(*_documentLoader, *_markdownRenderer);

        _configResolver     = std::make_unique<services::ConfigResolver>();
        _diffEngine         = std::make_unique<services::DiffEngine>();
        _promoteEngine      = std::make_unique<services::PromoteEngine>();
        _applyEngine        = std::make_unique<services::ApplyEngine>();
        _snapshotService    = std::make_unique<services::SnapshotService>(*_applyEngine);
        _scaffoldRepository = std::make_unique<services::ScaffoldRepository>();
        if (const char* home = std::getenv("HOME"))
        {
            const std::filesystem::path devdash{std::string(home) + "/.devdash"};
            _scaffoldRepository->SetScaffoldRoot(devdash / "scaffolds");
            _snapshotService->SetSnapshotRoot(devdash / "snapshots");
        }

        _projectSelectorPanel = std::make_unique<ui::ProjectSelectorPanel>(
            _window->Handle(),
            [this](const std::filesystem::path& path) { OnProjectSelected(path); });

        _window->Show();
        return true;
    }

    void App::OnProjectSelected(const std::filesystem::path& path)
    {
        _scaffoldDiffPanel.reset();
        _snapshotHistoryPanel.reset();
        _effectiveConfigPanel = std::make_unique<ui::EffectiveConfigPanel>(
            *_configResolver, *_documentPanelHost, core::Project{path});
        _appState = AppState::kViewingConfig;
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

            switch (_appState)
            {
            case AppState::kSelectingProject:
                _projectSelectorPanel->Render();
                break;

            case AppState::kViewingConfig:
                _effectiveConfigPanel->Render();
                _documentPanelHost->Render();
                if (_effectiveConfigPanel->WantsHistory())
                {
                    _snapshotHistoryPanel = std::make_unique<ui::SnapshotHistoryPanel>(
                        *_snapshotService, _effectiveConfigPanel->Project());
                    _appState = AppState::kSnapshotHistory;
                }
                else if (_effectiveConfigPanel->WantsScaffoldDiff())
                {
                    _scaffoldDiffPanel = std::make_unique<ui::ScaffoldDiffPanel>(
                        *_scaffoldRepository, *_diffEngine, *_promoteEngine,
                        *_applyEngine, *_snapshotService,
                        *_documentPanelHost, _effectiveConfigPanel->Project());
                    _appState = AppState::kScaffoldDiff;
                }
                else if (_effectiveConfigPanel->WantsBack())
                {
                    _effectiveConfigPanel.reset();
                    _appState = AppState::kSelectingProject;
                }
                break;

            case AppState::kScaffoldDiff:
                _scaffoldDiffPanel->Render();
                _documentPanelHost->Render();
                if (_scaffoldDiffPanel->WantsBack())
                {
                    _scaffoldDiffPanel.reset();
                    _appState = AppState::kViewingConfig;
                }
                break;

            case AppState::kSnapshotHistory:
                _snapshotHistoryPanel->Render();
                if (_snapshotHistoryPanel->WantsBack())
                {
                    _snapshotHistoryPanel.reset();
                    _appState = AppState::kViewingConfig;
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

            constexpr VkClearValue clearColor = {.color = {.float32 = {0.06f, 0.06f, 0.06f, 1.0f}}};

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
