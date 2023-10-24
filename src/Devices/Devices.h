#pragma once

#define DISK_SECTORS_PER_CYL 18
#define DISK_CYLS 24
#define DISK_HEADS_PER_CYL 4
#define DISK_SECTORS DISK_CYLS*DISK_SECTORS_PER_CYL

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include "../preload.h"

extern std::map<unsigned short, unsigned short> ports;

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
      //_push(regs.flags.all); FIXME TODO _push() problem idk
      //_push(regs.cs);
      //_push(regs.pc);
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
