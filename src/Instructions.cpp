#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <map>

#include "Instructions.h"
#include "Base.h"
#include "Utils.h"

#define DEFAULT_ARGS struct InstructionArgs args

namespace InstructionHandler {
  void _NOP(DEFAULT_ARGS) {
    regs.pc+=1;
    return;
  }

  void _HLT(DEFAULT_ARGS) {
    Processor.hlt = true;
    regs.pc+=1;
    return;
  }

  void NotImplemented(DEFAULT_ARGS) {
    unsigned char size = opcode_map[regs.ir].size;
    regs.pc+=size;
    return;
  }

  void _IN_al_dx(DEFAULT_ARGS) {
    // TODO FIXME
    regs.ax.al = 1;
  }

  namespace MOV {
    void _AL_imm8(struct InstructionArgs args) {
      //regs.ax = (regs.ax&AH) | args.imm8_value;
      regs.ax.al = args.imm8_value;
      regs.pc += 2;
    }
  }

  namespace CALL {
    void _rel16(struct InstructionArgs args) {
      regs.pc+=args.imm16_value;
      // TODO FIXME fazer a parada da stack e tal
    }
  }
}
