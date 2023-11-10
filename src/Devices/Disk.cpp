#include "Disk.h"

namespace Device {
  void Disk::handle_read_sector() {
    unsigned int lba = (ports[0x1F5] << 16) | (ports[0x1F4] << 8) | ports[0x1F3];
    unsigned short num_sectors = ports[0x1F2];
    unsigned short* addr_to_read = (unsigned short*)(m_location_in_memory + lba*512);
    ports[0x1F0] = (addr_to_read[last_byte*last_sector]);
    last_byte+=2;
    if(last_byte==512) {
      last_sector++;
      last_byte=0;
    }
    // TODO checar se chegamos no ultimo byte do ultimo setor
  }
} // namespace Device
