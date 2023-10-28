#include "ModRM.h"
ModRM decode_modrm(unsigned short value) {
  unsigned char ModR_M = value & 0x00FF;
  unsigned char Mod = (ModR_M & 0xC0) >> 6;
  unsigned char Reg_Opcode = (ModR_M & 0x38) >> 3;
  unsigned char RM = ModR_M & 0x07;
      
  bool is_register = Mod == 0b11;
  bool is_memory_address_indirect = Mod == 0b00;
  bool is_memory_address_with_one_byte_displacement = Mod == 0b01;
  bool is_memory_address_with_word_displacement = Mod == 0b10;

  return {ModR_M, Mod, Reg_Opcode, RM, is_register, is_memory_address_indirect, is_memory_address_with_one_byte_displacement, is_memory_address_with_word_displacement};
}
