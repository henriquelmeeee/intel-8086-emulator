#include "Devices.h"

#include "stdio.h"
#include "stdlib.h"
#include "vector"

#define DISK_SECTORS_PER_CYL 18
#define DISK_CYLS 24
#define DISK_HEADS_PER_CYL 4
#define DISK_SECTORS DISK_CYLS*DISK_SECTORS_PER_CYL

#define ATA_STATUS_PORT 0x1F7
#define ATA_COMMAND_PORT 0x1F7
#define ATA_DATA_PORT 0x1F0
#define ATA_SECTORS_COUNT 0x1F2
#define ATA_DRIVE_SELECT 0x1F6
#define ATA_READ_CMD 0x20
namespace Device {
  class Disk {
    private:
      std::vector<unsigned short> ports_in_use = {};
      int last_sector = 0;
      int last_byte = 0;

      char* m_location_in_memory = 0;
      unsigned short m_current_selected_drive = 0; // out ATA_DRIVE_SELECT, 0xE0
      unsigned short m_current_sectors_count = 0;
      unsigned short m_last_byte = 0;
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
        switch(ports[ATA_COMMAND_PORT]) {
          case ATA_READ_CMD:
            m_current_selected_drive = ports[ATA_DRIVE_SELECT];
            m_current_sectors_count = ports[ATA_SECTORS_COUNT];
            handle_read_sector();
            break;
          default:
            break;
        }
      }
      // Cada porto de dados pode ter um callback para 'in' e um callback para 'out'
      void callback_in_data_port() { // it will be called when 'IN xx, ATA_DATA_PORT'
        if(m_current_sectors_count) {
          m_last_byte += 2;
          if(m_last_byte == 512) {
            m_last_byte = 0;
            --m_current_sectors_count;
          }


        }
      }

  };
} // namespace Device
