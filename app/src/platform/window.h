#pragma once

#include <string_view>
#include <SDL3/SDL.h>

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

        bool PollEvent(SDL_Event& outEvent) const;
        void ProcessEvent(const SDL_Event& event);

        bool ShouldClose() const { return _shouldClose; }
        bool IsPaused()    const { return _paused; }

        bool ConsumeResizeFlag();

        void GetDrawableSize(int& outWidth, int& outHeight) const;
        void Show();

    private:
        SDL_Window* _window     = nullptr;
        bool _shouldClose       = false;
        bool _resized           = false;
        bool _paused            = false;
    };
}
