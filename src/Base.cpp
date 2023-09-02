#include "stdio.h"
#include "stdlib.h"
#include "iostream"
#include "Base.h"
#include "Utils.h"

void move_cursor(short x, short y) {
  std::cout.flush();
  printf("\033[%d;%dH", x+1, y+1);
  std::cout.flush();

  /* Simulate the cursor of text-mode */


  cursor_location = (y*VIDEO_COLUMNS) + x + VIDEO_MEMORY_BASE; // FIXME talvez precise incrementar em mais um por conta de cada caractere ocupar 2 bytes?

}

void dump_registers() {
  std::cout << "PC: 0x" << itoh(regs.pc) << "\t\tIR: 0x" << itoh(regs.ir) << "\t\tCS: 0x" << itoh(regs.cs) << "\n";
  std::cout << "AX: 0x" << itoh(regs.ax.ax) << "\t\tCX: 0x" << itoh(regs.cx.cx) << "\t\tDX: 0x" << itoh(regs.dx.dx) << "\n";
  std::cout << "BX: 0x" << itoh(regs.bx.bx) << "\n";
  std::cout << "SP: 0x" << itoh(regs.sp) << "\t\tBP: 0x" << itoh(regs.bp) << "\n";
  return;
}

void _push(short value) {
  // TODO overflow check
  regs.sp -= 2;
  *((unsigned short*)virtual_memory_base_address+(regs.ss*16)+regs.sp) = (unsigned short) value;
}

/*void cursor_update_byone() {
  // TODO podemos otimizar isso, evitando que façamos subtração em VIDEO_MEMORY_BASE toda hora
  // talvez criando outra variável relacionada ao cursor, mas sem contar o VIDEO_MEMORY_BASE
  // btw o código abaixo (calcular X e Y) é inútil por enquanto
  int y = (cursor_location-VIDEO_MEMORY_BASE)/VIDEO_WIDTH;
  int x = (cursor_location-VIDEO_MEMORY_BASE)%VIDEO_WIDTH;

  if(x>=VIDEO_WIDTH) {
    ++y; x=0;
  } else {
    ++x;
  }

  cursor_location+=2;

  return;
}*/

/*void write_char_on_memory(char ch) {
  *(virtual_memory_base_address+VIDEO_MEMORY_BASE+cursor_location) = ch;
  *(virtual_memory_base_address+VIDEO_MEMORY_BASE+cursor_location+1) = 0; // white
}*/


unsigned short* get_register_by_index(unsigned char index) {
  // TODO get register by index in "value_to_add", a menos que seja algo como add [imm16], value
  switch(index) {
    case 0:
      return reinterpret_cast<unsigned short*>(&regs.ax);
    case 1:
      return reinterpret_cast<unsigned short*>(&regs.cx);
    case 2:
      return reinterpret_cast<unsigned short*>(&regs.dx);
    case 3:
      return reinterpret_cast<unsigned short*>(&regs.bx);
    case 4:
      return &regs.sp;
    case 5:
      return &regs.bp;
    case 6:
      return &regs.si;
    case 7:
      return &regs.di;
    default:
      return &regs.di;
      // TODO FIXME throw CPU fault invalid opcode
  }
}

unsigned short get_register_value_by_index(unsigned char index) {
  return *get_register_by_index(index);
}

void jump_to(int offset) {
  regs.pc += (offset);
}


