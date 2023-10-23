#pragma once

typedef unsigned char byte;
typedef unsigned short word;

#define KB 1024
#define MB (KB*KB)

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <mutex>

extern std::mutex sdl_mutex;

template <typename I> std::string itoh(I w, size_t hex_len = sizeof(I)<<1) {
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len,'0');
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
        rc[i] = digits[(w>>j) & 0x0f];
    return rc;
}

extern std::map<unsigned char, struct InstructionInfo> opcode_map;

extern bool should_exit;
extern unsigned short current_memory_addr;

class Interruption;

class Processor {
  public:
    bool areInException = false;
    bool areInInterruption = false;

    std::queue<Interruption*> int_queue = {};

    bool hlt = false;
    char last_key = 0;
    bool keyboard_pendent_interrupt = false; // ? TODO FIXME why??

    Processor() {}
};

extern Processor CPU;

#include "Video/MainVideo.h"
#include "Instructions/Instructions.h"
#include "Devices/Devices.h"

extern unsigned char* virtual_memory_base_address;

extern unsigned long cursor_location;
inline void write_char_on_memory(char ch) {
  *(virtual_memory_base_address+cursor_location) = ch;
}
inline void cursor_update_byone() {
  cursor_location+=2;
}

struct FlagsRegister {
  word CF : 1;
  word rsvd : 1;
  word PF : 1;
  word rsvd2 : 1;
  word AF : 1;
  word rsvd3 : 1;
  word ZF : 1;
  word SF : 1;
  word TF : 1;
  word IF : 1;
  word DF : 1;
  word OF : 1;
  word IOPL : 2;
  word NT : 1;
  word rsvd4 : 1;
  word RF : 1;
  word VM : 1;
  word AC : 1;
  word VIF : 1;
  word VIP : 1;
  word ID : 1;
};

union Flags {
  FlagsRegister flags;
  word all;
};

struct Registers {
  union {
    struct {unsigned char al; unsigned char ah;};
    unsigned short ax;
  } ax;
  union {
    struct {unsigned char al; unsigned char ah;};
    unsigned short ax;
  } cx;
  union {
    struct {unsigned char al; unsigned char ah;};
    unsigned short ax;
  } dx;
  union {
    struct {unsigned char al; unsigned char ah;};
    unsigned short ax;
  } bx;
  word sp;
  word bp;
  word si;
  word di;
  word pc; 
  Flags flags;
  word cs;
  word ss;
  word ds;
  word es;
  unsigned char ir;
} extern regs;

/* FLAGS
 bits
      0->CF->Carry Flag
      1->Reserved
      2->PF->Parity Flag
      3->Reserved
      4->AF->Auxiliary Carry Flag
      5->Reserved
      6->Zero Flag
      7->Sign Flag
      8->Trap Flag (debugging purposes)
      9->IF->Interrupt Flag
      10->DF->Direction Flag
      11->OF->Overflow Flag
      12,13,14,15->IOPL->I/O Privilege Level Field

      NOW, x86-32BITS FLAGS RELATED, NOT RELEVANT IN THIS CASE!!!!!!!!!!!!!
      16->NT->Nested Task Flag
      17->Reserved
      18->RF->Resume Flag (debugging purposes)
      19->VM->Virtual 8086 mode
      20->AC->Allignment Check
      21->VIF->Virtual Interrupt Flag
      22->VIP->Virtual Interrupt Pending
      23->ID->ID flag
      24,25,26,27,28,29,30,31->Reserved
*/

#define CF (regs.flags.flags.CF)
#define ZF (regs.flags.flags.ZF)
#define PF (regs.flags.flags.PF)
#define SF (regs.flags.flags.SF)
#define IF (regs.flags.flags.IF)
#define OF (regs.flags.flags.OF)
#define AF (regs.flags.flags.AF)
#define DF (regs.flags.flags.DF)

extern void _push(short value);
extern void jump_to(int offset);
void dump_registers();
void move_cursor(short x, short y);

unsigned short* get_register_by_index(unsigned char index);
unsigned short get_register_value_by_index(unsigned char index);