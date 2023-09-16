#define OCL_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <mutex>
#include <sstream>

#include "Base.h"
#include "Utils.h"
#include "Instructions.h"

template <typename I> std::string _itoh(I w, size_t hex_len = sizeof(I)<<1) {
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len,'0');
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
        rc[i] = digits[(w>>j) & 0x0f];
    return rc;
}

class Screen : public olc::PixelGameEngine {

  public:
    Screen() {
      this->sAppName = "8086 emulator stats";
    }

    bool OnUserCreate() override {
      return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
      Clear(olc::BLUE);

      int line = 3;

      unsigned short _pc = regs.pc;
      int column = 2;

      // Primeiras três linhas
      DrawString(column, line, "PC: ", olc::WHITE, 1);
      DrawString(column+38, line, (_itoh(_pc)), olc::WHITE, 1);
      line+=10;
      DrawString(column, line, "AX: ", olc::WHITE, 1);
      DrawString(column+38, line, _itoh(regs.ax.ax), olc::WHITE, 1);
      line+=10;
      DrawString(column, line, "CX: ", olc::WHITE, 1);
      DrawString(column+38, line, _itoh(regs.cx.cx), olc::WHITE, 1);

      // Três colunas adicionais das três linhas
      column+=90;
      line = 3;
      DrawString(column, line, "SP: ", olc::WHITE, 1);
      DrawString(column+38, line, _itoh(regs.sp), olc::WHITE, 1);
      DrawString(column, line+10, "BP: ", olc::WHITE, 1);
      DrawString(column+38, line+10, _itoh(regs.bp), olc::WHITE, 1);
      DrawString(column, line+20, "DX: ", olc::WHITE, 1);
      DrawString(column+38, line+20, _itoh(regs.dx.dx), olc::WHITE, 1);
      line+=10;
      column = 2;





      line+=20;
      DrawString(2, line, "Memory dump (PC-based):", olc::WHITE, 1);
      line+=10;
      
      column = 52;

      for(int a = 0; a<4; a++) {
        unsigned short addr = _pc+a+10;
        DrawString(2, line, _itoh(addr), olc::WHITE, 1);
        for(int b = 0; b<5; b++) {
          unsigned short addr_content = *(virtual_memory_base_address+addr+b);
          DrawString(column, line, _itoh(addr_content), olc::WHITE, 1);
          column += 40;
        }
        line+=10;
        column = 52;
      }

      line+=10;
      DrawString(2, line, "Reading address:", olc::WHITE, 1);
      line+=10;
      DrawString(2, line, _itoh(current_memory_addr), olc::WHITE, 1);

      
      //DrawString(10, 10, "test", olc::WHITE, 2);

      return true;

    }

    bool OnUserDestroy() override {
      exit(0);
    }
};

void DebugScreenThread() {
  Screen screen;
  while(!should_exit){
  if(screen.Construct(256, 240, 4, 4)) {
    screen.Start();
  }}
  return;
}
