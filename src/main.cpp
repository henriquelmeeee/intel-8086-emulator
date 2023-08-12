/*
 * 16-bit x86 CPU emulator (Intel 8086)
 * it runs MS-DOS ;)
 * just for educational purposes.
 by https://henriquedev.com
*/

#include <iostream>
#include <fstream>

bool STEP_BY_STEP=false;

// TODO FIXME colocar isso num .h pra todos arquivos poderem usar
// inclusive o Instructions.h
// TODO FIXME colocar handler pra instrucao "iret"

class Processor {
  public:
    bool AreInException;
} Processor;


#include <stdlib.h>
#include <sys/mman.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <thread>
#include <chrono>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "Utils.h"

unsigned long iterations = 0;

#include <map>
#include "Instructions.h"
#include "Exceptions.h"

#include <boost/program_options.hpp>

std::map<unsigned char, struct InstructionInfo> opcode_map = {
  {0xE8, {3, InstructionHandler::CALL::_rel16, "CALL rel16"}},
  {0x04, {2, InstructionHandler::MOV::_AL_imm8, "ADD al, imm8"}},
  {0x72, {2, InstructionHandler::NotImplemented, "JB rel8"}},
  {0x90, {1, InstructionHandler::_NOP, "NOP"}},
  {0x2C, {2, InstructionHandler::NotImplemented, "SUB al, imm8"}}, // not sure!
  
  /* IN and OUT */
  {0xE4, {2, InstructionHandler::NotImplemented, "IN al, imm8"}},
  {0xEC, {1, InstructionHandler::_IN_al_dx, "IN al, dx"}},
};

/* Video-related */

unsigned long cursor_location = VIDEO_MEMORY_BASE;
const int FONT_WIDTH = 8;
const int FONT_HEIGHT = 16;

/* GDB protocol configuration */

/*int serverSocket, newSocket;
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
}*/

#include "./Base.h"
#include "./Instructions.h"

std::map<unsigned short, unsigned short> ports;

struct Registers regs;

void move_cursor(short x, short y) {
  std::cout.flush();
  printf("\033[%d;%dH", x+1, y+1);
  std::cout.flush();

  /* Simulate the cursor of text-mode */


  cursor_location = (y*VIDEO_COLUMNS) + x + VIDEO_MEMORY_BASE; // FIXME talvez precise incrementar em mais um por conta de cada caractere ocupar 2 bytes?

}

#define cout std::cout
#define flush std::flush

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

void dump_registers () {
  cout << "PC: 0x" << itoh(regs.pc) << "\t\tIR: 0x" << itoh(regs.ir) << "\t\tCS: 0x" << itoh(regs.cs) << "\n";
  cout << "AX: 0x" << itoh(regs.ax.ax) << "\t\tCX: 0x" << itoh(regs.cx.cx) << "\t\tDX: 0x" << itoh(regs.dx.dx) << "\n";
  cout << "BX: 0x" << itoh(regs.bx) << "\n";
  cout << "SP: 0x" << itoh(regs.sp) << "\t\tBP: 0x" << itoh(regs.bp) << "\n";
  return;
}

int main_clock_freq = 30;

void infinite_loop() {
    while(true);
}

void inline cursor_update_byone() {
  // TODO podemos otimizar isso, evitando que façamos subtração em VIDEO_MEMORY_BASE toda hora
  // talvez criando outra variável relacionada ao cursor, mas sem contar o VIDEO_MEMORY_BASE
  // btw o código abaixo (calcular X e Y) é inútil por enquanto
  int y = (cursor_location-VIDEO_MEMORY_BASE)/VIDEO_WIDTH;
  int x = (cursor_location-VIDEO_MEMORY_BASE)%VIDEO_WIDTH;

  if(x>=VIDEO_WIDTH) {
    ++y; x=0;
  } else {
    ++x;
  }

  cursor_location+=2;

  return;
}

void inline write_char_on_memory(char ch) {
  *(virtual_memory_base_address+VIDEO_MEMORY_BASE+cursor_location) = ch;
  *(virtual_memory_base_address+VIDEO_MEMORY_BASE+cursor_location+1) = 0; // white
}

void inline jump_to(int offset) {
  regs.pc += (offset);
}

unsigned short inline get_register_value_by_index(unsigned char index) {
  // TODO get register by index in "value_to_add", a menos que seja algo como add [imm16], value
  return 1;
}

void push(short value) {
  // TODO overflow check
  regs.sp -= 2;
  *((unsigned short*)virtual_memory_base_address+(regs.ss*16)+regs.sp) = (unsigned short) value;
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
    };
    // 1-byte opcodes
    //cout << "[DBG] regs.ir: " << itoh(regs.ir) << "\n";
    switch( (regs.ir) ) {
        case NOP: {
            cout << "nop\n";
            //++regs.pc;
            InstructionHandler::_NOP(args);
            exit(1); // TEMPORARY for debug
            RETURN;
        };
        default: {
            //cout << "[DBG] regs.ir>>8,1byte, NOTHING FOUND\n";
            break;
        };
    }

    // 2-byte opcodes test
    byte imm_value = *(virtual_memory_base_address+(regs.cs*16)+regs.pc+1);
    signed char offset_1byte = (signed char) imm_value; // conditional jmps
    switch( (regs.ir) ) {

      /* MOvs */

      case 0xB0: { // mov al, imm8
        //regs.ax = (regs.ax&AH) | imm_value;
        regs.ax.al = imm_value;
        regs.pc += 2;
        return {};
      }

      case 0xB1: { // mov cl, imm8
        //regs.cx = (regs.cx&AH) | imm_value;
        regs.cx.ch = imm_value;
        regs.pc += 2;
        return {};
      }

      case 0xB2: { // mov dl, imm8
        //regs.dx = (regs.dx&AH) | imm_value;
        regs.dx.dh = imm_value;
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
        //regs.ax = (regs.ax&AL) | (imm_value<<8);
        regs.ax.ah = args.imm8_value;
        regs.pc += 2;
        return {};
      }

      case 0xB5: { // mov ch, imm8
        //regs.cx = (regs.cx&AL) | (imm_value<<8);
        regs.cx.cl = args.imm8_value;
        regs.pc += 2;
        RETURN;
      }


      case 0xB6: { //mov dh, imm8
        //regs.dx = (regs.dx&AL) | (imm_value<<8);
        regs.dx.dl = args.imm8_value;
        regs.pc += 2;
        RETURN;
      }


      case 0xB7: { // mov bh, imm8
        regs.bx = (regs.bx&AL) | (imm_value<<8);
        regs.pc += 2;
        RETURN;
      }


      case INT: {
        // Interruptions
            
        cout << "regs.ir: " << itoh(regs.ir) << "\n";
        switch( args.imm8_value ) {
          case 0x10: { // BIOS VIDEO SERVICE
            //cout << "[DBG] BIOS VIDEO SERVICE\n";
            //switch( ((regs.ax)&AH)>>8 ) {
            switch(regs.ax.ah) {
              case 0x0e: { // DISPLAY CARACTER
                //cout << (char)((regs.ax)&AL) << flush;
                cout << (char)regs.ax.al << flush;
                regs.pc+= 2;
                
                //write_char_on_memory((char)(regs.ax&AL));
                write_char_on_memory((char)regs.ax.al);
                cursor_update_byone();
                RETURN;
              }
              case 0x02: { // CHANGE CURSOR
                //unsigned short line = ((regs.dx)&AH)>>8;
                //unsigned short column = (regs.dx)&AL;
                //unsigned short video_page = ((regs.bx)&AH)>>8;
                cout << "change cursor: todo: line,column,video_page, tirar &AH &AL e colocar regs.bx.bh regs.bx.bl\n";
                //move_cursor(line, column);
                regs.pc += 2;
                RETURN;
              };
          };
          break;
        };
   
        case 0x03: {
          exit(1);
        };

        case 0x13: { // read disk to memory (simulated)
          // TODO o codigo abaixo é para a funcao 0x002 (AH), mas existem mais funcoes q precisam ser tratadas
          CF = 0;
          cout << "INT 0x13\n";
          unsigned short sectors_to_read = regs.ax.al;
          unsigned short cyl_number = regs.cx.ch;
          unsigned short sector_base = regs.cx.cl;
          unsigned short head_number = regs.dx.dh;
          unsigned short drive_number = regs.dx.dl;
          unsigned long addr_dest = ((regs.es*16)+regs.bx) + (unsigned long)virtual_memory_base_address;

          if(devices->disks[drive_number]) {
            unsigned short* addr_disk = (unsigned short*)(devices->disks[drive_number]->addr);
            addr_disk+=(cyl_number * DISK_HEADS_PER_CYL + head_number)*DISK_SECTORS_PER_CYL + sector_base;
            
            // TODO 2 bytes por vez, mas podemos fazer 8 bytes por vez pra economizar ciclos
            
            cout << "SECTORS TO READ: " << sectors_to_read << "\n";
            cout << "VIRTUAL MEMORY ADDRESS TO WRITE: " << (unsigned long)((unsigned short*)addr_dest)-(unsigned long)virtual_memory_base_address;
            
            for(int sector = 0; sector < sectors_to_read ; sector++) {
              cout << "SECTOR: " << sector << "\n";
              for(int _byte = 0; _byte<256; _byte++) {

                int ACTUAL_SECTOR = sector+sector_base;
                unsigned short* actual_addr_buffer = addr_disk+ACTUAL_SECTOR*256+_byte;

                *((unsigned short*)addr_dest+ACTUAL_SECTOR*256+_byte) = *actual_addr_buffer;
                
                cout << "write to byte " << _byte*2 << " at " << (unsigned long)((unsigned short*)addr_dest+ACTUAL_SECTOR*256+_byte) << "\t";
                cout << "content: 0x" << itoh(*actual_addr_buffer) << "\n";
              }

            }
          } else {
            CF = 1;
          }
          regs.pc+=2;
          RETURN;
        }
      }
      break;
      }
  
      /* Conditional Jumps */


      case 0x77: { // ja/jnbe (>=)
        if(CF == 0 && ZF == 0) {
          jump_to(offset_1byte+2);
          return {};
        }
        regs.pc += 2;
        RETURN;
      }
      
      case 0x74: { // je (==)
        if(ZF == 1) {
          jump_to(offset_1byte+2);
          RETURN;
        }
        regs.pc+=2;
        RETURN;
      }

      case 0x75: { // jne (!=)
        if(ZF == 0) {
          jump_to(offset_1byte+2);
          RETURN;
        }
        regs.pc+=2;
        RETURN;
      }

      case 0x72: { // jb (!(>=))
        cout << "jb ";
        if(CF == 1) {
          cout << "true\n";
          jump_to(offset_1byte+2);
          RETURN;
        }
        cout << "false\n";
        regs.pc+=2;
        RETURN;
      }

      // TODO jnb (if cf == 0)
      // TODO jl (if sf != of)

      /* sums */

      case 0x04: { // add al, imm8
        unsigned char to_sum = offset_1byte;
        cout << "add al, " << to_sum << "\n";
        //unsigned int value = regs.ax&AL;
        unsigned int value = regs.ax.al;
        cout << "value of AL: " << value << "\n";
        unsigned int new_value = value + to_sum;
        cout << "new value of AL: " << new_value << "\n";
        
        //regs.ax = (regs.ax&AL)&0;
        regs.ax.al = 0;
        regs.ax.al = ((new_value)&0xFF);
        cout << "commited: " << (regs.ax.al) << "\n";

        CF = (new_value>0xFF);
        PF = (__builtin_parity(new_value&0xFF));
        AF = ((value ^ to_sum ^ new_value) & 0x10) != 0;
        ZF = (new_value&0xFF)==0;
        SF = ((new_value & 0x80)!=0);
        OF = ((value ^ -to_sum) & (value ^ new_value) & 0x80 != 0);

        regs.pc+=2;
        RETURN;
      }

      default: {
        break;
      };
    };
    
    /* 3-byte opcode tests */

    imm_value = (short) (*(virtual_memory_base_address+(regs.cs*16)+regs.pc+1));
    switch( (regs.ir) ) {
        
      case 0xB8: { // mov ax, imm16 
        regs.ax.ax = imm_value;
        regs.pc += 3;
        return {};
      };

      case 0xB9: { // mov cx, imm16
        regs.cx.cx = imm_value;
        regs.pc += 3;
        return {};
      };

      case 0xBA: { // mov dx, imm16
        regs.dx.dx = imm_value;
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
        regs.pc += imm_value+3;
        return {};
      }

      case 0xEB: { // jmp short in same-segment
        signed char offset = (signed char) (imm_value & 0xFF);
        jump_to(offset+2);
        RETURN;
      }

      case 0x08: { // add ax, imm16
        unsigned short to_sum = imm_value;
        cout << "add ax, " << to_sum << "\n";
        unsigned int value = regs.ax.ax;
        cout << "value of AX: " << value << "\n";
        unsigned int new_value = value + to_sum;
        cout << "new value of AX: " << new_value << "\n";
        
        regs.ax.ax = new_value;
        cout << "commited: " << (regs.ax.ax) << "\n";

        CF = (new_value>0xFFFF);
        PF = (__builtin_parity(new_value&0xFF));
        AF = ((value ^ to_sum ^ new_value) & 0x10) != 0;
        ZF = (new_value&0xFFFF)==0;
        SF = ((new_value & 0x8000)!=0);
        OF = ((value ^ -to_sum) & (value ^ new_value) & 0x8000 != 0);

        regs.pc+=2;
        RETURN;
      }
      
      /*
       * Now we need to decode opcode 0x00
       * We get an 1-byte memory address from a register
       * and 1-byte operand
       * so for example:
       * add byte [bx], al
       * is the same as:
       * 0x0007
       * where "0x07" is ModR/M 00000111 (bx, al, indirect address)
      */


      case 0x00: { // add addr_from_reg, reg
        unsigned char Mod = (offset_1byte&0xC0)>>6;
        unsigned char Reg_or_Opcode = (offset_1byte&0x38)>>3;
        unsigned char R_M = (offset_1byte&0x07);
        cout << "add [reg/addr], reg\n";
        if(Mod == 0) { // uses register as pointer (ex: add [bx], ax)
          cout << "mod 0\n";
          unsigned short pointer = 0;
          switch(R_M) {
            case 0: { // [BX + SI]
              pointer = (regs.ds*16) + regs.bx + regs.si;
              cout << "bx+si\n";
              break;
            }
            case 1: { // [BX + DI]
              pointer = (regs.ds*16) + regs.bx + regs.di;
              cout << "bx+di\n";
              break;
            }
            case 2: { // [BP + SI]
              pointer = (regs.ds*16) + regs.bp + regs.si;
              cout << "bp+si\n";
              break;
            }
            case 3: { // [BP + DI]
              pointer = (regs.ds*16) + regs.bp + regs.di;
              cout << "bp+di\n";
              break;
            }
            case 4: { // [SI] TODO
              break;
            }
            case 5: { // [DI] TODO
              break;
            }
            case 6: { // 16bits imm
              pointer = (unsigned short)*((char*)virtual_memory_base_address+regs.pc+2);
              // TODO special case here, the code needs to follow another routine.
              break;
            }
            case 7: { // [BX] TODO
              break;
            }
            default: {break;}
          }
          cout << "calculating\n";
          unsigned char value_at_pointer = *reinterpret_cast<unsigned char*>(&virtual_memory_base_address[pointer]);
          unsigned char value_to_add__reg = *reinterpret_cast<unsigned short*>(&virtual_memory_base_address[regs.pc+2]);
          value_to_add__reg = get_register_value_by_index(value_to_add__reg);
          unsigned char result = value_at_pointer + value_to_add__reg;

          cout << "result: " << (unsigned short)result << "\nat address: " << pointer << "\n";

          *reinterpret_cast<unsigned char*>(&virtual_memory_base_address[pointer]) = result;
          regs.pc+=2;
          RETURN;
          // TODO flags

        } else {
          RETURN;
        }
      }

      case 0x8B: {
      }

      default: {
        // flag Interruption Flag não afeta exceções da CPU
        cout << "\033[31mCPU Fault at 0x" << itoh(regs.pc) << "\033[0m\n";
        dump_registers();
        cout << "Instructions Counter: " << iterations << "\nCalling handler...\n";
        if(Processor.AreInException) {
          cout << "\033[31mDouble fault detected, shutting down...\033[0m\n";
          exit(1);
        }
        push(regs.cs);
        push(regs.pc);
        push(regs.flags.all);

        unsigned short code_segment = *((unsigned short*)virtual_memory_base_address+_INVALID_OPCODE*4);
        regs.cs = code_segment;
        unsigned short routine_addr = *((unsigned short*)virtual_memory_base_address+_INVALID_OPCODE*4+2);
        regs.pc = routine_addr;
        Processor.AreInException = true;
        cout << "PC is now 0x" << itoh(regs.pc) << "\n";
        return {};
      };
    }

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
      cout << "Registers:\n";
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

namespace Video {
#define VIDEO_REFRESH_RATE 1000 / 60

  void drawCharsOnRefresh(SDL_Renderer* renderer, SDL_Texture* fontTexture, const char* videoMemory) {
    //cout << "[Video] drawCharsOnRefresh\tDesenhando caracteres...\n";
    for(int y = 0; y < VIDEO_ROWS / FONT_HEIGHT; y++) {
      for(int x = 0; x < VIDEO_COLUMNS / FONT_WIDTH; x++) {
        char ch = videoMemory[y*(VIDEO_WIDTH/FONT_WIDTH)+x];
        cout << ch;
        // TODO render char in GUI
      }
    }
  }
  
  bool running=true;

  void refresh(const char* videoMemory) {
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
      std::cerr << "SDL_Init err: " << SDL_GetError() << std::endl;
      running = false;
      return;
    }
    
    SDL_Window* window = SDL_CreateWindow("SDL2 Simple Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, VIDEO_WIDTH, VIDEO_HEIGHT, SDL_WINDOW_SHOWN);
    if(window == nullptr) {
      std::cerr << "SDL_CreateWindow err: " << SDL_GetError() << std::endl;
      SDL_Quit();
      running=false;
      return;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == nullptr) {
      SDL_DestroyWindow(window);
      std::cerr << "SDL_CreateRenderer err: " << SDL_GetError() << std::endl;
      running=false;
      return;
    }

    SDL_Event event;

    SDL_Surface* fontSurface = IMG_Load("font.png");
    if (!fontSurface) {
        std::cerr << "IMG_Load Error: " << IMG_GetError() << std::endl;
        return;
    }
    SDL_Texture* fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
    SDL_FreeSurface(fontSurface);

    while (running) {
      while(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) {
          running = false;
          break;
        }
      }
      cout << "[Video] refresh\tAtualizando janela...";
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      SDL_RenderClear(renderer);
      cout << "\tJanela limpa...";

      SDL_Surface* fontSurface = IMG_Load("font.png");
      if(!fontSurface) {
        cout << "!fontSurface\n"; // TODO handle
      }
      SDL_Texture* fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
      /*SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
      SDL_Rect rect = {50,50,200,100};
      SDL_RenderFillRect(renderer, &rect); // apenas para teste*/
      
      drawCharsOnRefresh(renderer, fontTexture, videoMemory);
      SDL_RenderPresent(renderer);
      cout << "\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(VIDEO_REFRESH_RATE));
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
  }
}

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
        
        if(STEP_BY_STEP)
          wait_for_user();
        ++iterations;
        decode_and_execute(devices);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / main_clock_freq));
    }
}

extern "C" int main(int argc, char *argv[]) {
    /* Argumentos:
        * argv[1] = arquivo de disco *.img
        *
    */

      namespace po = boost::program_options;

      po::options_description desc("Allowed options");
      desc.add_options()
        ("breakpoint,bp", "breakpoint gerado no início da execução")
        ("master,m", po::value<std::string>(), "caminho para o disco master")
        ("slaves,disks", po::value<std::string>(), "discos adicionais");

      po::variables_map vm;
      po::store(po::parse_command_line(argc, argv, desc), vm);
      po::notify(vm);
      
      if(vm.count("breakpoint")) {
        STEP_BY_STEP = true;
        cout << "BREAKPOINT habilitado\n";
      }

      if(!vm.count("master")){
        cout << "Informe um disco para ser carregado\n";
        exit(1);
      }

      system("clear");
      
      /*serverSocket = 0*/;
      #ifdef CONNECT_BY_GDB
        if(!StartGDBCommunication())
          return -1;
      #endif
      virtual_memory_base_address = (byte*) mmap(NULL, 2*MB, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      cout << "Endereço base da memória virtual alocada (512MB): 0x" \
                  << itoh((unsigned long)virtual_memory_base_address) << "\n";
                    
      cout << "Carregando bootloader (setor 0) para memória no endereço-offset 0x7c00...\n";
      
      const char* master_param_location = vm["master"].as<std::string>().c_str();
      FILE* disk = std::fopen(master_param_location, "rb");

      FILE* second_disk = std::fopen("handle_cpu_fault", "rb"); // just for test
      if(!disk) {
        cout << "Erro ao ler arquivo"; return -1; }
      byte buffer[512];
      std::fread(buffer, sizeof(byte), 512, disk);
      for(int byte_ = 0; byte_ < 512; byte_++) {
        if(buffer[byte_] == 0)
          cout << ".";
        else
          cout << "!";
        *(virtual_memory_base_address+0x7c00+byte_) = buffer[byte_];
      }
      

      /* Agora, este é apenas uma representação "forçada" para lidar com CPU FAULT invalid opcode
       * na prática, temos que carregar os discos fornecidos em ordem
       * criando uma instância de "Disk" para cada um, e atribuindo endereços E/S
       * e, ao mesmo tempo, um ponteiro para seus dados na memória
       * mas, por enquanto, está bom assim
      */

      std::fread(buffer, sizeof(byte), 4, second_disk);
      std::fclose(second_disk);
      for(int byte_ = 0; byte_ < 4; byte_++) {
        *(virtual_memory_base_address+0x00FF+byte_) = buffer[byte_];
      }
      *((unsigned short*)virtual_memory_base_address+0x001A) = 0x00FF; // Handler para CPU FAULT INVALID_OPCODE
      cout << "\n";
        
      regs.pc = (unsigned short)0x7c00;
      regs.cs = 0;
      regs.ss = 0;
      regs.ds = 0;
      regs.es = 0; // TEMPORARY UNTIL WE MAKE MOVS OF SEGMENT REGISTERS (0x8E)
      regs.flags.all = 0;

      const char* videoMemory = (const char*) virtual_memory_base_address+VIDEO_MEMORY_BASE;

      std::thread refreshThread(Video::refresh, videoMemory);

      refreshThread.detach();

      Device::Keyboard *kb = new Device::Keyboard();
      Device::Disk *master = new Device::Disk(buffer); // TODO get addr of disk 0

      Device::Devices *devices = new Device::Devices(master, kb);
      auto wrapper = [&]() {start_execution_by_clock(devices);};

      std::thread execution_by_clock(wrapper);
      execution_by_clock.detach();

      while(Video::running);

      cout << "Programa finalizado\n";
      std::fclose(disk); // TEMPORARY, depois precisamos passar o FILE como argumento para o bgl de execucao

      return 0;
    return 0;
}
