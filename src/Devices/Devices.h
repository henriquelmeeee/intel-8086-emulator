#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include "../preload.h"

extern std::map<unsigned short, unsigned short> ports;

enum InterruptionType {
  KEYBOARD,
  MOUSE,
  UNKNOWN,
};

struct Interruption {
  enum InterruptionType type;
  void* interruption_object;
};

class KeyboardInterruption {
  public:
    char key;

    KeyboardInterruption(char _key) {
      key = _key;
      std::cout << "New Keyboard Interruption\n";
    };

    bool handle() {
      //_push(regs.flags.all); FIXME TODO _push() problem idk
      //_push(regs.cs);
      //_push(regs.pc);
      unsigned char* addr_dest = virtual_memory_base_address+38;
      unsigned long segment = *((unsigned short*)(addr_dest+2)); // endereços 40 e 41
      unsigned long addr_to_jump = *((unsigned short*)addr_dest);// endereços 38 e 39
      regs.pc = (segment << 4)+addr_to_jump;
      // TODO definir estado atual do processador para uma interrupção
      return true;
    };
};

