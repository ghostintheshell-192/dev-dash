#include "window.h"

#include <stdexcept>
#include <SDL3/SDL.h>

namespace dev_dash::platform
{
    Window::Window(std::string_view title, int width, int height)
    {
        _window = SDL_CreateWindow(
            title.data(),
            width,
            height,
            SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
        if (!_window)
            throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    }

    Window::~Window()
    {
        if (_window)
            SDL_DestroyWindow(_window);
    }

    bool Window::PollEvent(SDL_Event& outEvent) const
    {
        return SDL_PollEvent(&outEvent);
    }

    void Window::ProcessEvent(const SDL_Event& event)
    {
        switch (event.type)
        {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_QUIT:
            _shouldClose = true;
            break;
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_HIDDEN:
            _paused = true;
            break;
        case SDL_EVENT_WINDOW_RESTORED:
        case SDL_EVENT_WINDOW_SHOWN:
            _paused = false;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            _resized = true;
            break;
        default:
            break;
        }
    }

    bool Window::ConsumeResizeFlag()
    {
        if (!_resized)
            return false;
        _resized = false;
        return true;
    }

    void Window::GetDrawableSize(int& outWidth, int& outHeight) const
    {
        SDL_GetWindowSizeInPixels(_window, &outWidth, &outHeight);
    }

    void Window::Show()
    {
        SDL_ShowWindow(_window);
    }
}
