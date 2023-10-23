#pragma once

#include "../preload.h"
#include "../Devices/Devices.h"
#include "../Instructions/Instructions.h"
#include "../Exceptions.h"
#include "DebugScreen.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define VIDEO_WIDTH 80
#define VIDEO_HEIGHT 25
#define VIDEO_COLUMNS 80
#define VIDEO_ROWS 25

namespace Video {
    void drawCharsOnRefresh(SDL_Renderer* renderer, const unsigned short* videoMemory, TTF_Font* font);
    void refresh(const char* videoMemory);
}

