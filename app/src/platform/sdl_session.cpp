#include "sdl_session.h"

#include <stdexcept>
#include <SDL3/SDL.h>

namespace dev_dash::platform
{
    SdlSession::SdlSession()
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
            throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    SdlSession::~SdlSession()
    {
        SDL_Quit();
    }
}
