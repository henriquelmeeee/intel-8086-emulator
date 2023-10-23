#include "MainVideo.h"

/* Interruption-related with SDL2; Video too */

std::queue<struct Interruption> int_queue;

namespace Video {
#define VIDEO_REFRESH_RATE 1000 / 60

  void drawCharsOnRefresh(SDL_Renderer* renderer, const unsigned short* videoMemory, TTF_Font* font) {
    //cout << "[Video] drawCharsOnRefresh\tDesenhando caracteres...\n";
    for(int y = 0; y < VIDEO_ROWS; y++) {
      for(int x = 0; x < VIDEO_COLUMNS; x++) {
        char ch = videoMemory[y*VIDEO_COLUMNS+x];
        char str[2] = {ch, 0};

        SDL_Color textColor = {255, 255, 255, 255};

        SDL_Surface* surface = TTF_RenderText_Solid(font, str, textColor);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_Rect dst;
        dst.x = x * FONT_WIDTH;
        dst.y = y * FONT_HEIGHT;
        dst.w = FONT_WIDTH;
        dst.h = FONT_HEIGHT;
        SDL_RenderCopy(renderer, texture, NULL, &dst);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);

        //cout << ch;
      }
    }
  }
  
  bool running=true;

  void refresh(const char* videoMemory) {
    std::unique_lock<std::mutex> lock(sdl_mutex);
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
      std::cerr << "SDL_Init err: " << SDL_GetError() << std::endl;
      running = false;
      return;
    }
    

    SDL_Window* window = SDL_CreateWindow("Terminal", \
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, VIDEO_WIDTH * FONT_WIDTH, VIDEO_HEIGHT * FONT_HEIGHT, 0);

    if(window == nullptr) {
      std::cerr << "SDL_CreateWindow err: " << SDL_GetError() << std::endl;
      SDL_Quit();
      running=false;
      return;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == nullptr) {
      SDL_DestroyWindow(window);
      std::cerr << "SDL_CreateRenderer err: " << SDL_GetError() << std::endl;
      running=false;
      return;
    }

    SDL_Event event;

    TTF_Init();

    TTF_Font* font = TTF_OpenFont("fonts/VGA.ttf", VIDEO_HEIGHT);
    if(font == nullptr) {
      SDL_Log("Erro ao carregar a fonte: %s", TTF_GetError());
      exit(1);
    }
    lock.unlock();

    while (running) { // TODO colocar isso numa thread separada pode melhorar a latencia entre o teclado e o emulador
      while(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) {
          running = false;
          break;
        } else if(event.type == SDL_KEYDOWN) {
          unsigned char key = event.key.keysym.sym;
          // TODO tratamento especial para teclas de controle (modificação do are_shift_pressed, etc)
          CPU.last_key = key;
          CPU.keyboard_pendent_interrupt = true;
          int_queue.push({KEYBOARD, new KeyboardInterruption(key)});
        };
      }
      SDL_RenderClear(renderer);

      drawCharsOnRefresh(renderer, (const unsigned short*)videoMemory, font);
      SDL_RenderPresent(renderer);
      std::this_thread::sleep_for(std::chrono::milliseconds(VIDEO_REFRESH_RATE));
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    should_exit = true;
  }
}
