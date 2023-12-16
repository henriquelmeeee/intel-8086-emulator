#include "Disk.h"
#include "Devices.h"
#include "preload.h"

namespace Device {

  void Disk::regFirstCallback() {
    devices_callbacks[0x1F0] = [&](CallbackParameters params) {
      // TODO tbm lidar com codigo de erro
      // por ex em falhar ao ler memoria etc
      if(params.cb_operation_type == IN) {
        if(m_current_sectors_count) {
          m_last_byte += 2;
          if(m_last_byte == 512) {
            m_last_byte = 0;
            --m_current_sectors_count; //
          }
          return; // TODO retornar os 2 bytes lidos com read_memory() func
          // e checar se deu certo a leitura, claro
        }
      } else {
        // TODO escrita (acho q de 2 em 2 bytes tb)
      }
    };
  }

  void Disk::regSecondCallback() {
    devices_callbacks[0x1F1] = [&](CallbackParameters params) {
      // TODO return registro de erro
    };
  }

  void Disk::regThirdCallback() {
    devices_callbacks[0x1F2] = [&](CallbackParameters params) {

    };
  }

  void Disk::regFourthCallback() {
    devices_callbacks[0x1F7] = [&](CallbackParameters params) {
      // OUT: comandos
      // IN: status do disco, como se ta ocupado, se ocorreram erros, etc
      if(params.cb_operation_type == IN)  {

      } else {

      }
    };
  }

  void __handle_drive_select_disk(char drive_select) {
    switch(drive_select) {
      case 0:
        {
          // seleciona o primario
          break;
        }
      case 1:
        {
          // seleciona o secundario
          break;
        }
    }
  }

  void Disk::regFifthCallback() {
    devices_callbacks[0x1F6] = [&](CallbackParameters params) {
      if(params.cb_operation_type == IN) {
        // TODO retornar informacoes sobre o modo de operacao e a selecao do disco
      } else {
        char drive_select = params.cb_operation_type >> 7;
        __handle_drive_select_disk(drive_select);
      }
    };
  }

/*
  void Disk::handle_read_sector() {
    unsigned int lba = (ports[0x1F5] << 16) | (ports[0x1F4] << 8) | ports[0x1F3];
    unsigned short* addr_to_read = (unsigned short*)(m_location_in_memory + lba*512);
  }
*/
} // namespace Device
