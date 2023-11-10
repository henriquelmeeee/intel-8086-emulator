#include "Devices.h"

#include "stdio.h"
#include "stdlib.h"
#include "vector"

#define DISK_SECTORS_PER_CYL 18
#define DISK_CYLS 24
#define DISK_HEADS_PER_CYL 4
#define DISK_SECTORS DISK_CYLS*DISK_SECTORS_PER_CYL

namespace Device {
  class Disk {
    private:
      std::vector<unsigned short> ports_in_use = {};
      int last_sector = 0;
      int last_byte = 0;
      char* m_location_in_memory = 0;
    public:

      Disk() {
        std::cout << "[Disk()] disco desconhecido criado\n";
      }

      Disk(char* location_in_memory) : m_location_in_memory(location_in_memory){
        std::cout << "[Disk()] disco criado com conteúdo em 0x" << location_in_memory << "\n";
        ports_in_use = { // hardcodado temporariamente
          0x1F0,
          0x1F1,
          0x1F2,
          0x1F3,
          0x1F4,
          0x1F5,
          0x1F6,
          0x1F7
        };
      }

      void handle_read_sector();

      bool Refresh() {

        switch(ports[0x1F7]) {
          case 0x20:
            handle_read_sector();
            break;
          default:
            break;
        }
        
      }

  };
} // namespace Device
