#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <map>

#include "Instructions.h"
#include "Base.h"
#include "Utils.h"

namespace InstructionHandler {
  void _NOP(struct InstructionArgs args) {
    regs.pc+=1;
    return;
  }

  void NotImplemented(struct InstructionArgs args) {
    unsigned char size = opcode_map[regs.ir].size;
    regs.pc+=size;
    return;
  }

  namespace MOV {
    void _AL_imm8(struct InstructionArgs args) {
      regs.ax = (regs.ax&AH) | args.imm8_value;
      regs.pc += 2;
      return;
    }
  }

  namespace CALL {
    void _rel16(struct InstructionArgs args) {
      regs.pc+=args.imm16_value;
      // TODO FIXME fazer a parada da stack e tal
    }
  }
}
