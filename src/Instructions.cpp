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
    CPU.hlt = true;
    regs.pc+=1;
    return;
  }

  void NotImplemented(DEFAULT_ARGS) {
    unsigned char size = opcode_map[regs.ir].size;
    regs.pc+=size;
    return;
  }
  
  unsigned char* bits8_find_reg_by_index(unsigned char index) {
    return &regs.ax.al; // TODO FIXME temporary
  }

  void _ADD_regoraddr_8bits(DEFAULT_ARGS) {
    unsigned char byte_modrm = args.imm8_value;

    unsigned char first_operand_is_reg_or_memoryref = (byte_modrm) & 0xC0; // MOD
    unsigned char reg_or_opcode_extension = ((byte_modrm) & 0x38) >> 3; // DEST REG
    unsigned char second_operand_reg_or_addrmode = (byte_modrm) & 0x07; // ORIG REG 
    
    unsigned char Mod = first_operand_is_reg_or_memoryref;
    unsigned char Reg_or_Opcode = reg_or_opcode_extension;
    unsigned char R_M = second_operand_reg_or_addrmode;

    if(Mod == 0b00000011) {
      // TWO 8-bit REGISTERS
      unsigned char dest_reg_index = (reg_or_opcode_extension >> 3) & 0x07;
      unsigned char src_reg_index = second_operand_reg_or_addrmode & 0x07;

      unsigned char* dest_reg = bits8_find_reg_by_index(dest_reg_index);
      unsigned char* src_reg = bits8_find_reg_by_index(src_reg_index);

      *(dest_reg) += *(src_reg);
      
      unsigned char result = *(dest_reg);

      if(result == 0)
        ZF = 1;
      // TODO continue this


    } else {
      std::cout << "MOD not implemented yet\n";
    }


    regs.pc += 3;
  }

  void _IN_al_dx(DEFAULT_ARGS) {
    // TODO FIXME implementar
    regs.ax.al = 1;
  }

  namespace MOV {
    void _AL_imm8(DEFAULT_ARGS) {
      //regs.ax = (regs.ax&AH) | args.imm8_value;
      regs.ax.al = args.imm8_value;
      regs.pc += 2;
    }

    void _BH_imm8(DEFAULT_ARGS) { // 0xB7
      regs.bx.bl = args.imm8_value;
      regs.pc+=2;
    }

    void _DH_imm8(DEFAULT_ARGS) { // 0xB6
      regs.dx.dh = args.imm8_value;
      regs.pc += 2;
    }

    void _CH_imm8(DEFAULT_ARGS) { // 0xB5
      regs.cx.ch = args.imm8_value;
      regs.pc += 2;
    }

    void _AX_imm16(DEFAULT_ARGS) { // 0xB8
      regs.ax.ax = args.imm16_value;
      regs.pc += 3;
    }

    void _AH_imm8(DEFAULT_ARGS) { // 0xB4
      regs.ax.ah = args.imm8_value;
      regs.pc += 2;
    }

    void _CL_imm8(DEFAULT_ARGS) {
      regs.cx.cl = args.imm8_value;
      regs.pc += 2;
    }

  }

  namespace CALL {
    void _rel16(DEFAULT_ARGS) {
      std::cout << "called CALL::_rel16\n";
      regs.pc+=args.imm16_value+2;
      // TODO FIXME fazer a parada da stack e tal
    }
  }
}
