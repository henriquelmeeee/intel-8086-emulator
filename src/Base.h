#pragma once

#ifndef BASE
#define BASE

typedef unsigned short word;

#include <stdlib.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>

template <typename I> std::string itoh(I w, size_t hex_len = sizeof(I)<<1) {
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len,'0');
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
        rc[i] = digits[(w>>j) & 0x0f];
    return rc;
}

extern std::map<unsigned char, struct InstructionInfo> opcode_map;

class Processor {
  public:
    bool areInException;
    bool areInInterruption;

    bool hlt;

    Processor() {
      this->areInException = false;
      this->hlt = false;
    }
};
extern Processor CPU;

#define VIDEO_WIDTH 80
#define VIDEO_HEIGHT 25
#define VIDEO_COLUMNS 80
#define VIDEO_ROWS 25

extern unsigned char* virtual_memory_base_address;

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

union __ax {
  struct {unsigned char al; unsigned char ah;};
  unsigned short ax;
};

union __cx {
  struct {unsigned char cl; unsigned char ch;};
  unsigned short cx;
};

union __dx {
  struct {unsigned char dl; unsigned char dh;};
  unsigned short dx;
};

union __bx {
  struct {unsigned  char bl; unsigned char bh;};
  unsigned short bx;
};

struct Registers {
    /* General-Purposes registers */
    //_ax ax;
    word ax_DEPRECATED;
    __ax ax;
    __cx cx;
    __dx dx;
    __bx bx;
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

extern void _push(short value);

#define DISK_SECTORS_PER_CYL 18
#define DISK_CYLS 24
#define DISK_HEADS_PER_CYL 4
#define DISK_SECTORS DISK_CYLS*DISK_SECTORS_PER_CYL

extern std::map<unsigned short, unsigned short> ports;

void _push(short value);
void inline cursor_update_byone();
void inline jump_to(int offset);
unsigned short inline get_register_value_by_index(unsigned char index);
void dump_registers();
void move_cursor(short x, short y);

enum InterruptionType {
  KEYBOARD,
  MOUSE,
  UNKNOWN,
};

struct Interruption {
  enum InterruptionType type;
  void* interruption_object;
};

class KeyboardInterruption {
  public:
    char key;

    KeyboardInterruption(char _key) {
      key = _key;
      std::cout << "New Keyboard Interruption\n";
    };

    bool handle() {
      _push(regs.flags.all);
      _push(regs.cs);
      _push(regs.pc);
      unsigned char* addr_dest = virtual_memory_base_address+38;
      unsigned long segment = *((unsigned short*)(addr_dest+2)); // endereços 40 e 41
      unsigned long addr_to_jump = *((unsigned short*)addr_dest);// endereços 38 e 39
      regs.pc = (segment << 4)+addr_to_jump;
      // TODO definir estado atual do processador para uma interrupção
      return true;
    };
};

namespace Device {


  class Disk {
    public:
      unsigned char* addr;
      unsigned long long size;
                                                        // DEFAULTS;
      /*unsigned int data_port;                             // 0x1F0
      unsigned int error_and_resources_port;              // 0x1F1, ...
      unsigned int count_sector_port;
      unsigned int sector_number_port;
      unsigned int low_cyl_port;
      unsigned int drive_head_port;
      unsigned int status_command_port;                   // 0x1F7*/
                                                          // WE WILL USE ports[x]

      std::vector<unsigned short> ports_in_use;
      unsigned char data_buffer[2];

      std::string lastError;
      // TODO criar o resto dos buffer

      Disk(unsigned char* location) {
        std::cout << "Creating new Disk\n";
        ports_in_use = {
          0x1F0,
          0x1F1,
          0x1F2,
          0x1F3,
          0x1F4,
          0x1F5,
          0x1F6,
          0x1F7
        };
        this->addr = location;
        lastError = "Unknown error";
      }
      int last_sector;
      int last_byte;
      bool Refresh() {
        switch(ports[0x1F7]) {
          case 0x20:
          {
            // READ SECTOR
            unsigned int lba = (ports[0x1F5] << 16) | (ports[0x1F4] << 8) | ports[0x1F3];
            unsigned short num_sectors = ports[0x1F2];
            unsigned short* addr_to_read = (unsigned short*)(addr + lba*512);
            ports[0x1F0] = (addr_to_read[last_byte*last_sector]);
            last_byte+=2;
            if(last_byte==512) {
              last_sector++;
              last_byte=0;
            }
            // TODO checar se chegamos no ultimo byte do ultimo setor
          };
          default:
          {
            break;
          };
        };
        return true;
      }

      std::string getLastError() {
        return this->lastError;
      }

      bool inline writeToData(unsigned short data) {
        ports[0x1F0] = data;
        return true;
      }

      bool inline setSectorCount(unsigned short sector) {
        ports[0x1F2] = sector;
        return true;
      }

      bool inline setHead(unsigned short head) {
        return false;
      }

      unsigned short inline readStatusPort() {
        return ports[0x1F7];
      }

      bool inline setCommandPort(unsigned short command) {
        ports[0x1F7] = command;
        return true;
      }

  };

  class Keyboard {
    public:
      unsigned char* addr;
      unsigned long long size;


      unsigned short data_port; // 0x60
      unsigned short command_and_status_port; // 0x64
  
      std::vector<unsigned short> ports_in_use;
      Keyboard() {
        std::cout << "Creating new PS/2 Keyboard\n";
        ports_in_use = {0x60, 0x64};
      }

      bool Refresh() {
        return true;
      }
  };

  class Devices {
    public:
      std::vector<Disk*> disks;
      Keyboard* keyboard_PS2;

      Devices(Disk* master, Keyboard* kb) {
        std::cout << "Initializing devices\n";
        this->keyboard_PS2 = kb;
        this->disks.push_back(master);
      }
  };

}
#endif
