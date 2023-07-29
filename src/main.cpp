/*
 * 16-bit x86 CPU emulator (Intel 8086)
 * it runs MS-DOS ;)
 * just for educational purposes.
 by https://henriquedev.com
*/

#include <iostream>
#include <stdlib.h>
#include <sys/mman.h>

#include <thread>
#include <chrono>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define CONNECT_BY_GDB

/* GDB protocol configuration */

#define PORT 1234

int serverSocket, newSocket;
char data_buffer[1024];
struct sockaddr_in serverAddr;
struct sockaddr_storage serverStorage;
socklen_t addr_size;

bool StartGDBCommunication() {
  serverSocket = socket(PF_INET, SOCK_STREAM, 0);

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  if(listen(serverSocket, 1)==0)
    std::cout << "Waiting for GDB";
  else
    return false;

  addr_size = sizeof serverStorage;
  newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);

  return true;
}

#include "./Base.h"
#include "./Instructions.h"

struct Registers regs;

#define cout std::cout

typedef unsigned char byte;
typedef unsigned short word;

#define KB 1024
#define MB (1024*1024)

byte *virtual_memory_base_address;

template <typename I> std::string itoh(I w, size_t hex_len = sizeof(I)<<1) {
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len,'0');
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
        rc[i] = digits[(w>>j) & 0x0f];
    return rc;
}

int main_clock_freq = 5; // 5 hertz 

void infinite_loop() {
    while(true);
}

extern "C" ExecutionState decode_and_execute() {
    /*
        * We will try to detect the operational code of the actual instruction 
        * First we will try to decode one-byte code
        * If we found nothing, we will try to decode 2-byte instructions 
        * starting by register moving
    */
    
    regs.ir = htons(regs.ir);
    // 1-byte opcodes
    //cout << "[DBG] regs.ir: " << itoh(regs.ir) << "\n";
    switch ((regs.ir) >> 8) {
        case NOP: {
            //cout << "[DBG] NOP found\n";
            ++regs.pc;
            return {};
        };
        default: {
            //cout << "[DBG] regs.ir>>8,1byte, NOTHING FOUND\n";
            break;
        };
    }

    // 2-byte opcodes test
    byte imm_value = *(virtual_memory_base_address+(regs.cs*16)+regs.pc+1);
    switch( (regs.ir) >> 8 ) {

        case 0xB0: { // mov al, imm8
          regs.ax = (regs.ax&AH) | imm_value;
          regs.pc += 2;
          return {};
        }

        case 0xB1: { // mov cl, imm8
          regs.cx = (regs.cx&AH) | imm_value;
          regs.pc += 2;
          return {};
        }

        case 0xB2: { // mov dl, imm8
          regs.dx = (regs.dx&AH) | imm_value;
          regs.pc += 2;
          return {};
        }

        case 0xB3: { // mov bl, imm8
          regs.bx = (regs.bx&AH) | imm_value;
          regs.pc += 2;
          return {};
        }

        /* AH (high) movs */

        case 0xB4: { //mov ah, imm 8
          regs.ax = (regs.ax&AL) | (imm_value<<8);
          regs.pc += 2;
          return {};
        }

        case 0xB5: { // mov ch, imm8
          regs.cx = (regs.cx&AL) | (imm_value<<8);
          regs.pc += 2;
          return {}; 
        }

        case 0xB6: { //mov dh, imm8
          regs.dx = (regs.dx&AL) | (imm_value<<8);
          regs.pc += 2;
          return {};
        }

        case 0xB7: { // mov bh, imm8
          regs.bx = (regs.bx&AL) | (imm_value<<8);
          regs.pc += 2;
          return {};
        }

        case INT: {

            // Interruptions
            
            //cout << "regs.ir: " << itoh(regs.ir) << "\n";
            switch( (regs.ir)&AL ) {
              case 0x10: { // BIOS VIDEO SERVICE
                //cout << "[DBG] BIOS VIDEO SERVICE\n";
                switch( ((regs.ax)&AH)>>8 ) { // AH
                    case 0x0e: { // DISPLAY CARACTER
                        cout << (char)((regs.ax)&AL);
                        cout << "sexo";
                        regs.pc+= 2;
                        return {};
                    }
                    default: {
                        cout << "?";
                        regs.pc += 2;
                        return {};
                    };
                };
                break;
              };
   
              case 0x03: {
                while(true);
              };
                
            }
            break;
        };
        default: {
            break;
        };
    };
    
    /* 3-byte opcode tests */

    imm_value = (short) htons(*(virtual_memory_base_address+(regs.cs*16)+regs.pc+1));
    switch( (regs.ir)>>8 ) {
        
      case 0xB8: { // mov ax, imm16 
        regs.ax = imm_value;
        regs.pc += 3;
        return {};
      };

      case 0xB9: { // mov cx, imm16
        regs.cx = imm_value;
        regs.pc += 3;
        return {};
      };

      case 0xBA: { // mov dx, imm16
        regs.dx = imm_value;
        regs.pc += 3;
        return {};
      }

      case 0xBB: { // mov bx, imm16
        regs.bx = imm_value;
        regs.pc += 3;
        return {};
      }

      case 0xBC: { // mov sp, imm16
        regs.sp = imm_value;
        regs.pc += 3;
        return {};
      }

      case 0xBD: { // mov bp, imm16
        regs.bp = imm_value;
        regs.pc += 3;
        return {};
      }

      case 0xBE: { // mov si, imm16
        regs.si = imm_value;
        regs.pc += 3;
        return {};
      }

      case 0xBF: { // mov di, imm16
        regs.di = imm_value;
        regs.pc += 3;
        return {};
      }

        /* Stack-related */

      case 0xE8: { // call in same-segment (code-segment)
        imm_value = (signed short) imm_value;
        regs.sp -= 2;
        unsigned short* sp_ptr = (unsigned short*)(virtual_memory_base_address+(regs.ss*16)+regs.sp);
        *sp_ptr = regs.pc+3;
        regs.pc += imm_value;
        return {};
      }

      case 0xEB: { // jmp short in same-segment
        imm_value = (signed char) (imm_value>>8);
        regs.pc+=imm_value;
        return {};
      }

      default: {
        cout << "CPU Fault";
        while(true);
        return {};
      };
    }

    return {};

}

const char* supported_features_gdb_response = "$#00";

extern "C" void start_execution_by_clock() {
    while(true) {
        //cout << "Execução de RIP em 0x" << itoh(regs.pc) << "\n";
        if(serverSocket != 0) {
          while(true) {
            memset(data_buffer, 0, sizeof(data_buffer));
            ssize_t numBytesRcvd = recv(newSocket, data_buffer, sizeof(data_buffer), 0);
            cout << "[GDB]\n" << data_buffer << "\n";
            if(numBytesRcvd == 0) {
              cout << "Connection closed by the client\n";
              return;
            }
            if(strncmp(data_buffer, "+$qSupported", 12) == 0) {
              send(newSocket, supported_features_gdb_response, strlen(supported_features_gdb_response), 0);
            } else if (data_buffer[1] == 'p') {
              //GDBCommunication:handle_p_command(((word*)&regs)[atoi(data_buffer+2)]);

            }
          }
        }
        word instruction_offset = (regs.cs*16) + regs.pc;
        regs.ir = *((short*)(virtual_memory_base_address+regs.cs+regs.pc));
        //cout << "Opcode: " << itoh(regs.ir) << "\n";
        decode_and_execute();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / main_clock_freq));
    }
}

extern "C" int main(int argc, char *argv[]) {
    /* Argumentos:
        * argv[1] = arquivo de disco *.img
        *
    */
    if(argc < 2) {
        cout << "Faltam argumentos";
        return -1;
    } else {
      serverSocket = 0;
      #ifdef CONNECT_BY_GDB
        if(!StartGDBCommunication())
          return -1;
      #endif
      virtual_memory_base_address = (byte*) mmap(NULL, 512*MB, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      cout << "Endereço base da memória virtual alocada (512MB): 0x" \
                  << itoh((unsigned long)virtual_memory_base_address) << "\n";
                    
      cout << "Carregando bootloader (setor 0) para memória no endereço-offset 0x7c00...\n";
        
      FILE* disk = std::fopen("source", "rb");
      if(!disk) {
        cout << "Erro ao ler arquivo"; return -1; }
        byte buffer[512];
        std::fread(buffer, sizeof(byte), 512, disk);
        std::fclose(disk); // TEMPORARY, depois precisamos passar o FILE como argumento para o bgl de execucao
        for(int byte_ = 0; byte_ < 512; byte_++) {
          if(buffer[byte_] == 0)
            cout << ".";
          else
            cout << "!";
          *(virtual_memory_base_address+0x7c00+byte_) = buffer[byte_];
        }
        cout << "\n";
        
        regs.pc = 0x7c00;
        regs.cs = 0;
        regs.ss = 0;
        regs.ds = 0;
        
        start_execution_by_clock();
        
        
        
    }
    
    return 0;
}
