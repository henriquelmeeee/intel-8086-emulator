#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <mutex>
#include <sstream>

#include "Base.h"
#include "Utils.h"

void DebugScreenThread() {
  //SDL_Init(SDL_INIT_VIDEO);
  //TTF_Init();
  std::unique_lock<std::mutex> lock(sdl_mutex);

  SDL_Window *window = SDL_CreateWindow("Debug Terminal", 100, 100, 640, 480, 0);
  if(window == nullptr)
    std::cout << "WINDOW NULLPTR\n";
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
  if(renderer == nullptr)
    std::cout << "RENDERER NULLPTR\n";

  TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 4);
  SDL_Color color = {255, 255, 255};

  SDL_Event event; 

  SDL_Rect destRect;
  destRect.x = 10;
  destRect.y = 10;
  destRect.w = 300;
  destRect.h = 100;

  lock.unlock();
  while(true) {
    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_QUIT)
        break;
    }

    std::stringstream ss;
    ss << "Registers: \n";

    std::string text = ss.str();

    SDL_Surface *surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_RenderPresent(renderer);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    SDL_Delay(16);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_CloseFont(font);

  //TTF_Quit();
  //SDL_Quit();
}
