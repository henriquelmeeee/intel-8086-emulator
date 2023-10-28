#pragma once

#ifndef __MODRM
#define __MODRM

#include "preload.h"

struct ModRM {
  unsigned char ModR_M;
  unsigned char Mod;
  unsigned char Reg_Opcode;
  unsigned char RM;
  bool is_register;
  bool is_memory_address_indirect;
  bool is_memory_address_with_one_byte_displacement;
  bool is_memory_address_with_word_displacement;
};

ModRM decode_modrm(unsigned short value);

#endif
