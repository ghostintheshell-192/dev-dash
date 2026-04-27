// dev-dash C++/ImGui PoC — Step 1: bootstrap.
// Goal of this file at this stage: prove the toolchain (CMake + CPM + Conan-less
// + system SDL3/Vulkan + ImGui via source) compiles and links end-to-end.
//
// No window is opened yet — that's Step 2 (porting Germen's Disegnatore.cpp
// into a stripped-down renderer.cpp). Here we just smoke-test the build.

#include <cstdio>

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

#include "imgui.h"

int main(int /*argc*/, char ** /*argv*/)
{
    std::printf("dev-dash C++/ImGui PoC — bootstrap step\n");
    std::printf("ImGui version: %s\n", IMGUI_VERSION);

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::printf("SDL_Init(SDL_INIT_VIDEO) failed: %s\n", SDL_GetError());
        return 1;
    }
    std::printf("SDL3 initialized\n");

    uint32_t apiVersion = 0;
    if (vkEnumerateInstanceVersion(&apiVersion) == VK_SUCCESS)
    {
        std::printf("Vulkan instance API: %u.%u.%u\n",
                    VK_VERSION_MAJOR(apiVersion),
                    VK_VERSION_MINOR(apiVersion),
                    VK_VERSION_PATCH(apiVersion));
    }
    else
    {
        std::printf("vkEnumerateInstanceVersion failed\n");
    }

    ImGui::CreateContext();
    std::printf("ImGui context created\n");
    ImGui::DestroyContext();

    SDL_Quit();

    std::printf("Bootstrap OK\n");
    return 0;
}
