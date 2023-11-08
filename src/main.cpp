#include "preload.h"

bool STEP_BY_STEP = false;

#include <stdlib.h>
#include <sys/mman.h>
#include <thread>
#include <chrono>
#include <string.h>
#include <map>

#include <boost/program_options.hpp>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <fstream>

#define NI InstructionHandler::NotImplemented

Registers regs = {};
Processor CPU;
void dump_registers() {
  std::cout << "dump_registers() todo";
}

unsigned long amount_of_memory = 2*MB;
void dump_vm_state_to_file() {
  // TODO talvez fazer dump de memoria e registradores após qualquer crash de CPU?
  std::cout << "[dump_memory_to_file()] starting the dump...";
  FILE* file = fopen("./vm.dump", "wb");
  if(file == NULL) {
    perror("File open failed");
    exit(1);
  }

  // VMState struct into file
    
  vm_state vm_state_buffer = vm_state();
  vm_state_buffer.vm_memory_size = amount_of_memory;
  vm_state_buffer.vm_mode = (char)1;

  fwrite(&vm_state_buffer, sizeof(vm_state), 1, file);

  // Memory dump

  fwrite(virtual_memory_base_address, amount_of_memory, 1, file);

  // Registers state
  // TODO a fazer.
  exit(0);
}

unsigned long cursor_location = 0;

void CLI(InstructionArgs args) {
  IF = 0;
  regs.pc += 1;
}

void jump_to(int offset) {
  regs.pc += offset;
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

  /* INC */
  {0x40, {1, InstructionHandler::INC16, "INC ax"}},
  {0x41, {1, InstructionHandler::INC16, "INC cx"}},
  {0x42, {1, InstructionHandler::INC16, "INC dx"}},
  {0x43, {1, InstructionHandler::INC16, "INC bx"}},
  {0x44, {1, InstructionHandler::INC16, "INC sp"}},
  {0x45, {1, InstructionHandler::INC16, "INC bp"}},
  {0x46, {1, InstructionHandler::INC16, "INC si"}},
  {0x47, {1, InstructionHandler::INC16, "INC di"}},
  
  {0xFF, {2, InstructionHandler::_INC_DEC_CALL, "INC/DEC/CALL"}}
};

byte *virtual_memory_base_address;
int main_clock_freq = 10;

ExecutionState decode_and_execute(Device::Devices* devices) {
  struct InstructionArgs args = {
    (unsigned char)*(virtual_memory_base_address+(regs.cs*16)+regs.pc),    
    (unsigned char)*(virtual_memory_base_address+(regs.cs*16)+regs.pc+1),
    (unsigned short)*(virtual_memory_base_address+(regs.cs*16)+regs.pc+1),
    devices,
  };

  if(opcode_map[regs.ir].handler != nullptr) {
    opcode_map[regs.ir].handler(args);
  } else {
    //throw_cpu_fault(invalid_opcode)
    std::cout << "insn not found\n";
    while(true);
  }


}

// wait_for_user() related

unsigned long iterations = 0;
char user_buffer[32];
void inline wait_for_user() {
  std::cout << "\nBreakpoint\n\nCycles: " << iterations << "\nInstruction: " << opcode_map[regs.ir].name << "\n\n";
  while(true) {
    std::cout << "Commands:\n\tni\t-> next instruction\n\tq\t-> quit\n\tdr\t-> dump registers\n\trd\t-> read memory address\n\twr\t-> write memory address" << "\n> ";
  
    std::cin >> user_buffer;
    unsigned short address_to_read = atoi(&(user_buffer[2]));
    std::cout << "\n";
    if(strcmp(user_buffer, "ni") == 0) {
      break;
    } else if (strcmp(user_buffer, "dr") == 0){
      std::cout << "Registers (big-endian converted):\n";
      dump_registers();
    } else if (strncmp(user_buffer, "rd", 2) == 0) {
      std::cout << "Address to read: " << address_to_read;
      short value = ((short) *((char*)virtual_memory_base_address+address_to_read));
      std::cout << "\nValue (signed): " << value;
      std::cout << "\nValue (unsigned): " << (unsigned short)value;
      std::cout << "\nInstruction (if valid): " << opcode_map[value].name;
      std::cout << "\n";
    } else if (strncmp(user_buffer, "wr", 2) == 0){
      std::cout << "Address to write: " << address_to_read;
      std::cout << "\nValue to write: > ";
      short value;
      std::cin >> value;
      *((char*)virtual_memory_base_address+address_to_read) = value;
      std::cout << "Success!\n";
    } else {
      exit(1);
    }
  }
}

bool should_exit = false;


unsigned short* get_register_by_index(unsigned char index) {
  // TODO get register by index in "value_to_add", a menos que seja algo como add [imm16], value
  switch(index) {
    case 0:
      return reinterpret_cast<unsigned short*>(&regs.ax);
    case 1:
      return reinterpret_cast<unsigned short*>(&regs.cx);
    case 2:
      return reinterpret_cast<unsigned short*>(&regs.dx);
    case 3:
      return reinterpret_cast<unsigned short*>(&regs.bx);
    case 4:
      return &regs.sp;
    case 5:
      return &regs.bp;
    case 6:
      return &regs.si;
    case 7:
      return &regs.di;
    default:
      return &regs.di;
      // TODO FIXME throw CPU fault invalid opcode
  }
}

unsigned short get_register_value_by_index(unsigned char index) {
  return *get_register_by_index(index);
}

void start_execution_by_clock(Device::Devices *devices) {
  while(true) {
    word instruction_offset = (regs.cs*16) + regs.pc;

    regs.ir = *((unsigned char*)(virtual_memory_base_address+(regs.cs*16)+regs.pc));
    std::cout << "regs.ir: " << (void*)regs.ir << '\n';

    for(auto disk : devices->disks) {
      if(!disk->Refresh()) {
        // o disco irá definir seu código de erro . 
      }
    }

  // FIFO model

  // TODO é realmente necessário checar "areInException"?
  // interrupções aninhadas seriam melhores, apenas teríamos que adicionar um contador 
  // pra evitar tirar o "cpuAreInInterruption" cedo demais
  // enfim, tem que melhorar isso no geral
#if 0
    if( (!CPU.int_queue.empty() && IF) && !CPU.areInException && !CPU.areInInterruption ) {
      if(CPU.hlt)
        CPU.hlt = false;
      Interruption* _int = CPU.int_queue.front();
      if(_int->type == KEYBOARD) {
        std::cout << "Calling handler of Keyboard interruption";
        KeyboardInterruption* handler = reinterpret_cast<KeyboardInterruption*>(_int->interruption_object);
        handler->handle();
      }
      CPU.int_queue.pop();
    }
#endif
    if(STEP_BY_STEP)
      wait_for_user();
    ++iterations;
    if(!CPU.hlt)
      decode_and_execute(devices);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / main_clock_freq));
  }
}


int main(int argc, char *argv[]) {
  //system("/bin/clear");
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
    std::cout << "[main] Breakpoint is enabled\n";
  }

  if(!vm.count("master")) {
    std::cout << "[main] ERROR: 'master' disk needs to be informed";
    exit(1);
  }
  
  virtual_memory_base_address = (byte*) mmap(NULL, amount_of_memory, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  std::cout << "[main] base of allocated VM memory: " << virtual_memory_base_address << "\n";
  std::cout << "[main] Loading bootloader\tTODO: Load BIOS instead\n";
  
  const char* master_param_location = vm["master"].as<std::string>().c_str();
  FILE* disk = std::fopen(master_param_location, "rb");

  if(!disk) {
    std::cout << "[main] Error while trying to read master disk";
    exit(1);
  }

  byte buffer[512];
  std::fread(buffer, sizeof(byte), 512, disk);
  for(int byte_ = 0; byte_ < 512; byte_++) {
    if(buffer[byte_] == 0)
      std::cout << ".";
    else
      std::cout << "!";
    *(virtual_memory_base_address+0x7c00+byte_) = buffer[byte_];
  }

  regs.pc = 0;
  regs.cs = 0x07c0;
  regs.ss = 0;
  regs.ds = 0x07c0;
  
  const char* videoMemory = (const char*) virtual_memory_base_address+VIDEO_MEMORY_BASE_ADDRESS;

  std::thread refreshThread(Video::refresh, videoMemory);
  refreshThread.detach();
  std::cout << "[main] Initializing devices\n";

  Device::Keyboard *kb = new Device::Keyboard();
  Device::Disk *master = new Device::Disk(buffer); // TODO get addr of disk 0

  Device::Devices *devices = new Device::Devices(master, kb);
  
  auto wrapper = [&]() {start_execution_by_clock(devices);};
  std::thread execution_by_clock(wrapper);
  execution_by_clock.detach();

  std::thread debug_screen(DebugScreenThread);
  debug_screen.detach();
      
  while(!should_exit);

  std::cout << "Program finished\n";
}
