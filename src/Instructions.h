#pragma once

#ifndef INST
#define INST

#include <string>
#include <stdlib.h>

struct InstructionArgs { 
  unsigned char opcode;
  unsigned char imm8_value;
  unsigned short imm16_value;
};

struct InstructionInfo {
  unsigned char size;
  void (*handler)(struct InstructionArgs);
  std::string name;

  InstructionInfo() : size(0), handler(nullptr), name("") {}
  InstructionInfo(int i, void (*h)(struct InstructionArgs), std::string d)
    : size(i), handler(h), name(d) {}
};


namespace InstructionHandler {
  void _NOP(struct InstructionArgs args);
  void NotImplemented(struct InstructionArgs args);
  namespace MOV {
    void _AL_imm8(struct InstructionArgs args);
  }
  namespace CALL {
    void _rel16(struct InstructionArgs args);
  }
}

// One-byte instructions
#define NOP 0x90

// Two-byte instructions
#define INT 0xCD

#endif
