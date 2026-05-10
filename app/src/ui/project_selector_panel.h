#pragma once

#include <array>
#include <filesystem>
#include <functional>
#include <string>

#include <SDL3/SDL.h>

namespace dev_dash::ui
{
    class ProjectSelectorPanel
    {
    public:
        using OnSelectedCallback = std::function<void(const std::filesystem::path&)>;

        ProjectSelectorPanel(SDL_Window* window, OnSelectedCallback onSelected);

        void Render();

    private:
        static void SDLCALL DialogCallback(void* userdata,
                                           const char* const* filelist,
                                           int filter);
        void TryConfirm();

        SDL_Window*        _window;
        OnSelectedCallback _onSelected;
        std::array<char, 512> _pathBuffer{};
        std::string        _pendingPath;
        bool               _hasPendingPath = false;
        std::string        _errorMessage;
    };
}
