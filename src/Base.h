#ifndef BASE
#define BASE

typedef unsigned short word;

#include <stdlib.h>
#include <iostream>
#include <string>
#include <map>

extern std::map<unsigned char, struct InstructionInfo> opcode_map;

#include "Instructions.h"

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


union _ax {
  word ah:8;
  word al:8;
  word ax;
};

struct Registers {
    /* General-Purposes registers */
    //_ax ax;
    word ax;
    word cx;
    word dx;
    word bx;
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
    word ir;
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

enum ExecutionState {
    SUCCESS,
    ERROR,
};

// LITTLE ENDIAN!!!!!!!
#define AH 0xFF00
#define AL 0x00FF

namespace Instruction {
  #define NOP 0x90
}

class Disk {
  public:
    unsigned char* addr;
    unsigned long long size;
                                                        // DEFAULTS;
    unsigned int data_port;                             // 0x1F0
    unsigned int error_and_resources_port;              // 0x1F1, ...
    unsigned int count_sector_port;
    unsigned int sector_number_port;
    unsigned int low_cyl_port;
    unsigned int drive_head_port;
    unsigned int status_command_port;                   // 0x1F7

    unsigned char data_buffer[2];
    // TODO criar o resto dos buffer

    Disk() {
      std::cout << "Creating new Disk\n";


    }
};

#endif
