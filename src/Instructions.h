#pragma once

#ifndef INST
#define INST

#include <string>
#include <stdlib.h>
#include "Utils.h"
#include "Base.h"
#include "Devices.h"

#define _DEFAULT_ARGS struct InstructionArgs
#define DEFAULT_ARGS _DEFAULT_ARGS args

struct InstructionArgs { 
  unsigned char opcode;
  unsigned char imm8_value;
  unsigned short imm16_value;
  Device::Devices* devices;
};

struct InstructionInfo {
  unsigned char size;
  void (*handler)(_DEFAULT_ARGS);
  std::string name;

  InstructionInfo() : size(0), handler(nullptr), name("") {}
  InstructionInfo(int i, void (*h)(_DEFAULT_ARGS), std::string d)
    : size(i), handler(h), name(d) {}
};


namespace InstructionHandler {
  void _NOP(DEFAULT_ARGS);
  void _HLT(DEFAULT_ARGS);
  void _INT(DEFAULT_ARGS);
  void NotImplemented(DEFAULT_ARGS);
  void _IN_al_dx(DEFAULT_ARGS);
  void _ADD_regoraddr_8bits(DEFAULT_ARGS);
  void _JMP_short(DEFAULT_ARGS);
  namespace MOV {
    void _RM16_R16(DEFAULT_ARGS);
    void _AL_imm8(DEFAULT_ARGS);
    void _BH_imm8(DEFAULT_ARGS);
    void _DH_imm8(DEFAULT_ARGS);
    void _CH_imm8(DEFAULT_ARGS);
    void _AX_imm16(DEFAULT_ARGS);
    void _SI_imm16(DEFAULT_ARGS);
    void _DI_imm16(DEFAULT_ARGS);
    void _AH_imm8(DEFAULT_ARGS);
    void _SP_imm16(DEFAULT_ARGS);
    void _BX_imm16(DEFAULT_ARGS);
    void _DL_imm8(DEFAULT_ARGS);
    void _CL_imm8(DEFAULT_ARGS);
  }
  namespace CALL {
    void _rel16(DEFAULT_ARGS);
  }
}

// One-byte instructions
#define NOP 0x90

// Two-byte instructions

#endif
