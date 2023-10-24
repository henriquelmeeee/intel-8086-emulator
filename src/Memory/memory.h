#pragma once

#include "../preload.h"

inline unsigned long __read_word(unsigned short address, unsigned char segment) {
  current_memory_addr = address;
  return *((unsigned short*)(virtual_memory_base_address+(segment*16)+address)); // FIXME *16 or just segment?
}

inline unsigned char __read_byte(unsigned short address, unsigned char segment) {
  current_memory_addr = address;
  return *((unsigned  char*)(virtual_memory_base_address+(segment*16)+address)); // same question
}

inline bool __write_word(unsigned short address, unsigned char segment, unsigned char value) {
  current_memory_addr = address;
  *((unsigned char*)virtual_memory_base_address+(segment*16)+address) = value;
  return true;
}

inline bool __write_dword(unsigned short address, unsigned char segment, unsigned short value) {
  current_memory_addr = address;
  *((unsigned short*)virtual_memory_base_address+(segment*16)+address) = value;
  return true;
}

