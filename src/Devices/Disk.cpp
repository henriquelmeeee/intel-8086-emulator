#include "Disk.h"

namespace Device {
  void Disk::handle_read_sector() {
    unsigned int lba = (ports[0x1F5] << 16) | (ports[0x1F4] << 8) | ports[0x1F3];
    unsigned short* addr_to_read = (unsigned short*)(m_location_in_memory + lba*512);
  }
} // namespace Device
