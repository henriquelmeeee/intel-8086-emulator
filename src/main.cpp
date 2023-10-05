/*
 * 16-bit x86 CPU emulator (Intel 8086)
 * it runs MS-DOS ;)
 * just for educational purposes.
 by https://henriquedev.com
*/


// TODO FIXME colocar o Device::devices para Instructions.cpp poder usar
// Fazer Base.cpp com as funcs principais tipo write_char_to_video_memory algo assim

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "MainVideo.h"

#include <iostream>
#include <queue>
#include <fstream>

bool STEP_BY_STEP=false;
bool should_exit = false;

// TODO FIXME colocar isso num .h pra todos arquivos poderem usar
// inclusive o Instructions.h
// TODO FIXME colocar handler pra instrucao "iret"

#include <stdlib.h>
#include <sys/mman.h>

#include <thread>
#include <chrono>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "Utils.h"

#include <map>
#include "Devices.h"
#include "Instructions.h"
#include "Exceptions.h"

#include <boost/program_options.hpp>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>

#include "DebugScreen.h"

std::mutex sdl_mutex;

#define NI InstructionHandler::NotImplemented

unsigned long iterations = 0;

void CLI(InstructionArgs args){
  IF = 0;
  regs.pc += 1;
}

std::map<unsigned char, struct InstructionInfo> opcode_map = {
  {0xE8, {3, InstructionHandler::CALL::_rel16, "CALL rel16"}},
  /* ADD, SUB */
  {0x04, {2, NI, "ADD al, imm8 (NI)"}},
  {0x2C, {2, NI, "SUB al, imm8 (NI)"}}, // not sure!
  
  /* JMPs */
  {0xE9, {3, NI, "JMP rel16 (NI)"}},
  {0xEB, {2, InstructionHandler::_JMP_short, "JMP rel8"}},
  {0x72, {2, NI, "JB rel8 (NI)"}},
  {0x74, {2, InstructionHandler::_JMP_if_equals, "JE rel8"}},
  {0x07, {1, NI, "POP es (NI)"}},

  {0x90, {1, InstructionHandler::_NOP, "NOP"}},
  
  /* IN and OUT */
  {0xE4, {2, NI, "IN al, imm8 (NI)"}},
  {0xEC, {1, InstructionHandler::_IN_al_dx, "IN al, dx"}},

  {0xF4, {1, InstructionHandler::_HLT, "HLT"}},
  {0xAC, {1, InstructionHandler::_LODSB, "LODSB"}},
  {0xFA, {1, CLI, "CLI"}},
  {0xCD, {2, InstructionHandler::_INT, "INT imm8"}},

  /* MOVs */
  {0x89, {2, InstructionHandler::MOV::_RM16_R16, "MOV r/m16, r16"}},
  {0x8A, {2, InstructionHandler::MOV::_R8_RM8, "MOV r8, r/m8"}},
  
  {0xB8, {3, InstructionHandler::MOV::_AX_imm16, "MOV ax, imm16"}},
  {0xBC, {3, InstructionHandler::MOV::_SP_imm16, "MOV sp, imm16"}},
  {0xBB, {3, InstructionHandler::MOV::_BX_imm16, "MOV bx, imm16"}},
  {0xBE, {3, InstructionHandler::MOV::_SI_imm16, "MOV si, imm16"}},
  {0xBF, {3, InstructionHandler::MOV::_DI_imm16, "MOV di, imm16"}},

  {0xB4, {2, InstructionHandler::MOV::_AH_imm8, "MOV ah, imm8"}},
  {0xB0, {2, InstructionHandler::MOV::_AL_imm8, "MOV al, imm8"}},
  {0xB5, {2, InstructionHandler::MOV::_CH_imm8, "MOV ch, imm8"}},
  {0xB1, {2, InstructionHandler::MOV::_CL_imm8, "MOV cl, imm8"}},
  {0xB6, {2, InstructionHandler::MOV::_DH_imm8, "MOV dh, imm8"}},
  {0xB7, {2, InstructionHandler::MOV::_BH_imm8, "MOV bh, imm8"}},
  {0xB2, {2, InstructionHandler::MOV::_DL_imm8, "MOV dl, imm8"}},

  {0x8E, {3, NI, "MOV Sreg, r/m16 (NI)"}},

  {0x00, {3, InstructionHandler::_ADD_regoraddr_8bits, "ADD reg_or_addr, reg"}},
  {0x01, {3, InstructionHandler::_ADD_regoraddr_16bits, "ADD reg_or_addr, reg"}},

  {0x3C, {2, InstructionHandler::CMP::_al_imm8, "CMP al, imm8"}},
  {0xC3, {1, InstructionHandler::_RET, "RET"}},

  {0x31, {2, NI, "XOR reg1, reg2 (NI)"}},
  {0x33, {2, NI, "XOR reg2, reg1 (NI)"}},
};

/* Video-related */

unsigned long cursor_location = VIDEO_MEMORY_BASE;
const int FONT_WIDTH = 8;
const int FONT_HEIGHT = 16;

#include "Base.h"
#include "Instructions/Instructions.h"

std::map<unsigned short, unsigned short> ports;

struct Registers regs;
Processor CPU;

#define cout std::cout
#define flush std::flush

typedef unsigned char byte;
typedef unsigned short word;

#define KB 1024
#define MB (1024*1024)

byte *virtual_memory_base_address;

int main_clock_freq = 10; //4770000;

inline void infinite_loop() {
    while(true);
}

extern "C" ExecutionState decode_and_execute(Device::Devices* devices) {
    /*
        * We will try to detect the operational code of the actual instruction 
        * First we will try to decode one-byte code
        * If we found nothing, we will try to decode 2-byte instructions 
        * starting by register moving
    */
    
    struct InstructionArgs args = {
      *(virtual_memory_base_address+(regs.cs*16)+regs.pc),
      (unsigned char)*(virtual_memory_base_address+(regs.cs*16)+regs.pc+1),
      (unsigned short)*(virtual_memory_base_address+(regs.cs*16)+regs.pc+1),
      devices,
    };

    if(!(opcode_map[regs.ir].handler == nullptr)) {
      opcode_map[regs.ir].handler(args);
      RETURN;
    } else {
      cout << "Instruction not found\n"; // TODO throw error invalid opcode
      infinite_loop();
    }

   /*
      default: {
        // flag Interruption Flag não afeta exceções da CPU
        cout << "\033[31mCPU Fault at 0x" << itoh(regs.pc) << "\033[0m\n";
        dump_registers();
        cout << "Instructions Counter: " << iterations << "\nCalling handler...\n";
        if(CPU.areInException) {
          cout << "\033[31mDouble fault detected, shutting down...\033[0m\n";
          exit(1);
        }
        _push(regs.cs);
        _push(regs.pc);
        _push(regs.flags.all);

        unsigned short code_segment = *((unsigned short*)virtual_memory_base_address+_INVALID_OPCODE*4);
        regs.cs = code_segment;
        unsigned short routine_addr = *((unsigned short*)virtual_memory_base_address+_INVALID_OPCODE*4+2);
        regs.pc = routine_addr;
        CPU.areInException = true;
        cout << "PC is now 0x" << itoh(regs.pc) << "\n";
        return {};
      };
    }
    */
    return {};

}

const char* supported_features_gdb_response = "$#00";

char user_buffer[32];

void inline wait_for_user() {
  cout << "\nBreakpoint\n\nCycles: " << iterations << "\nInstruction: " << opcode_map[regs.ir].name << "\n\n";
  while(true) {
    cout << "Commands:\n\tni\t-> next instruction\n\tq\t-> quit\n\tdr\t-> dump registers\n\trd\t-> read memory address\n\twr\t-> write memory address" << "\n> ";
  
    std::cin >> user_buffer;
    unsigned short address_to_read = atoi(&(user_buffer[2]));
    cout << "\n";
    if(strcmp(user_buffer, "ni") == 0) {
      break;
    } else if (strcmp(user_buffer, "dr") == 0){
      cout << "Registers (big-endian converted):\n";
      dump_registers();
    } else if (strncmp(user_buffer, "rd", 2) == 0) {
      cout << "Address to read: " << address_to_read;
      short value = ((short) *((char*)virtual_memory_base_address+address_to_read));
      cout << "\nValue (signed): " << value;
      cout << "\nValue (unsigned): " << (unsigned short)value;
      cout << "\nInstruction (if valid): " << opcode_map[value].name;
      cout << "\n";
    } else if (strncmp(user_buffer, "wr", 2) == 0){
      cout << "Address to write: " << address_to_read;
      cout << "\nValue to write: > ";
      short value;
      std::cin >> value;
      *((char*)virtual_memory_base_address+address_to_read) = value;
      cout << "Success!\n";
    } else {
      exit(1);
    }
  }
}

const int _CHAR_WIDTH = VIDEO_WIDTH / VIDEO_COLUMNS;
const int _CHAR_HEIGHT = VIDEO_HEIGHT / VIDEO_ROWS;

extern "C" void start_execution_by_clock(Device::Devices *devices) {
    while(true) {
        //cout << "Execução de RIP em 0x" << itoh(regs.pc) << "\n";
        /*if(serverSocket != 0) {
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
        }*/
        word instruction_offset = (regs.cs*16) + regs.pc;
        regs.ir = *((unsigned char*)(virtual_memory_base_address+regs.cs+regs.pc));
        //cout << "Opcode: " << itoh(regs.ir) << "\n";
        
        for(auto disk : devices->disks) {
          if(!(disk->Refresh())) {
            cout << "Disk error: " << disk->getLastError() << "\n";
          }
        }

        // FIFO model
        
        if( ((!(int_queue.empty())) && IF) && !CPU.areInException && !CPU.areInInterruption) {
          if(CPU.hlt)
            CPU.hlt = false;
          Interruption _int = int_queue.front();
          if(_int.type == KEYBOARD) {
            cout << "Calling handler of Keyboard Interruption\n";
            KeyboardInterruption* handler = reinterpret_cast<KeyboardInterruption*>(_int.interruption_object);
            handler->handle();
          }
          // TODO chamar interrupção adequada
          // seria bom, em cada interruption_object, ter uma rotina para lidar com o handler de interrupção,
          //int_queue.front()->interruption_object->handle();
          int_queue.pop();
        }
        
        if(STEP_BY_STEP)
          wait_for_user();
        ++iterations;
        if(!CPU.hlt)
          decode_and_execute(devices);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / main_clock_freq));
    }
}

// Every device will have an E/S port associated ("allocated")

extern "C" int main(int argc, char* argv[]) {
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("breakpoint,bp", "breakpoint at start")
    ("master,m", po::value<std::string>(), "master disk path")
    ("slaves,disks", po::value<std::string>(), "other disks");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if(vm.count("breakpoint")) {
    STEP_BY_STEP = true;
    cout << "[main] Breakpoint enabled\n";
  }

  if(!vm.count("master")) {
    cout << "[main] ERROR: 'master' disk needs to be informed";
    exit(1);
  }

  system("/bin/clear");

  virtual_memory_base_address = (byte*) mmap(NULL, 2*MB, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  cout << "[main] virtual_memory_base_addr: " << virtual_memory_base_address << "\n";
  cout << "[main] Loading bootloader\tTODO: Load BIOS instead\n";
  
  const char* master_param_location = vm["master"].as<std::string>().c_str();
  FILE* disk = std::fopen(master_param_location, "rb");

  //FILE* second_disk = std::fopen("handle_cpu_fault", "rb"); // just for test
  
  if(!disk) {
    cout << "Error while trying to rad master disk"; return -1; }

  byte buffer[512];
  std::fread(buffer, sizeof(byte), 512, disk);
  for(int byte_ = 0; byte_ < 512; byte_++) {
    if(buffer[byte_] == 0)
      cout << ".";
    else
      cout << "!";
    *(virtual_memory_base_address+0x7c00+byte_) = buffer[byte_];
  }

 /*
 
      \/\* Agora, este é apenas uma representação "forçada" para lidar com CPU FAULT invalid opcode
       * na prática, temos que carregar os discos fornecidos em ordem
       * criando uma instância de "Disk" para cada um, e atribuindo endereços E/S
       * e, ao mesmo tempo, um ponteiro para seus dados na memória
       * mas, por enquanto, está bom assim
      \*\/

      std::fread(buffer, sizeof(byte), 4, second_disk);
      std::fclose(second_disk);
      for(int byte_ = 0; byte_ < 4; byte_++) {
        *(virtual_memory_base_address+0x00FF+byte_) = buffer[byte_];
      }
      *((unsigned short*)virtual_memory_base_address+0x001A) = 0x00FF; // Handler para CPU FAULT INVALID_OPCODE
      cout << "\n";
        
 */

  regs.pc = (unsigned short)0x7c00;
  regs.cs = 0;
  regs.ss = 0;
  regs.ds = 0x7c0; // O PADRÃO SERIA 0X00
  regs.es = 0; // TEMPORARY UNTIL WE MAKE MOVS OF SEGMENT REGISTERS (0x8E)
  regs.flags.all = 0;

  const char* videoMemory = (const char*) virtual_memory_base_address+VIDEO_MEMORY_BASE;

  std::thread refreshThread(Video::refresh, videoMemory);

  refreshThread.detach();

  cout << "[main] Initializing devices 'keyboard' & 'master disk'\n";

  Device::Keyboard *kb = new Device::Keyboard();
  Device::Disk *master = new Device::Disk(buffer); // TODO get addr of disk 0

  Device::Devices *devices = new Device::Devices(master, kb);
  
  auto wrapper = [&]() {start_execution_by_clock(devices);};
  std::thread execution_by_clock(wrapper);
  execution_by_clock.detach();

  std::thread debug_screen(DebugScreenThread);
  debug_screen.detach();
      
  while(!should_exit);

  cout << "Program finished\n";
  std::fclose(disk); // TEMPORARY, depois precisamos passar o FILE como argumento para o bgl de execucao
      
  return 0;
}