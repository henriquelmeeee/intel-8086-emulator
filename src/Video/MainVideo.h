#pragma once

#include "../Utils.h"
#include "../Devices.h"
#include "../Instructions/Instructions.h"
#include "../Exceptions.h"
#include "../DebugScreen.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace Video {
    void drawCharsOnRefresh(SDL_Renderer* renderer, const unsigned short* videoMemory, TTF_Font* font);

    void refresh(const char* videoMemory);
}

