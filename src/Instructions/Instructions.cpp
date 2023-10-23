#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <map>
#include "../preload.h"

// TODO
// o registrador de código é o padrão usado para cálculo de offset
// se o programa quiser usar baseado no registrador de dados
// (regs.ds)
// ele vai precisar explicitar isso, e então o OPCODE mudará
// mas não lidarei com isso no momento

// FIXME o find_reg_by_index() ta retornando um ponteiro invalido

#include "Instructions.h"

unsigned short current_memory_addr = 0;

void __set_flags(signed short value) {
  ZF = (value == 0);
  SF = (value < 0);
  // OF needs to be manually updated
  //PF = check_parity(value & 0xFF);
}

namespace InstructionHandler {
  void _NOP(DEFAULT_ARGS) {
    regs.pc+=1;
    return;
  }

  void _HLT(DEFAULT_ARGS) {
    CPU.hlt = true;
    regs.pc+=1;
  }

  void _LODSB(DEFAULT_ARGS) {
    std::cout << "DS flag: " << regs.ds << "; *16: " << regs.ds*16 << "\n";
    //regs.ax.al = __read_word(regs.si, regs.ds*16);
    regs.ax.al = *(virtual_memory_base_address+(regs.ds*16)+regs.si);
    if(!DF)
      regs.si++;
    else
      regs.si--;
    regs.pc += 1;
  }

  void _RET(DEFAULT_ARGS) {
    regs.pc = __read_word(regs.sp, regs.ss << 4);
    //regs.pc = *((unsigned short*)(virtual_memory_base_address+(regs.ss << 4)+regs.sp));
    regs.sp -= 2;
  }

  void _print(DEFAULT_ARGS) {
    std::cout << (char)regs.ax.al << std::flush;
    regs.pc+= 2;
  }

  void _change_video_cursor(DEFAULT_ARGS) {
    //unsigned short line = ((regs.dx)&AH)>>8;
    //unsigned short column = (regs.dx)&AL;
    //unsigned short video_page = ((regs.bx)&AH)>>8;
    //move_cursor(line, column);
    regs.pc += 2;
  }
  
  

  void _INT(DEFAULT_ARGS) {
    unsigned char interrupt_number = args.imm8_value;

    switch(interrupt_number) {
        // TODO interrupções são da BIOS, não do emulador
        // por enquanto a emulação é suficiente
      case 0x10: /* Video-related interruption */ 
        {
          
          switch(regs.ax.ah) {
            case 0x0e:
              {
                std::cout << "int print char\n";
                _print(args);
                write_char_on_memory((char)regs.ax.al);
                cursor_update_byone();
                break;
              };

            case 0x02:
              {
                _change_video_cursor(args);
                break;
              };
            default:
              {
                std::cout << "[DBG] Warning: Video-Interrupt Number " << (unsigned int)interrupt_number << " not implemented yet\n";
                regs.pc += 2;
              };
          }
         break;

        };
      
      case 0x03:
        {
          //wait_for_user();
          regs.pc += 2;
          std::cout << "TODO case 0x03 INT passar o wait_for_user() para Instructions.cpp através de Base.h\n";
          break;
        };
      
      case 0x13: // Write disk data to RAM
        {
          // TODO o codigo abaixo é para a funcao 0x002 (AH), mas existem mais funcoes q precisam ser tratadas
          CF = 0;
          unsigned short sectors_to_read = regs.ax.al;
          unsigned short cyl_number = regs.cx.ch;
          unsigned short sector_base = regs.cx.cl;
          unsigned short head_number = regs.dx.dh;
          unsigned short drive_number = regs.dx.dl;
          unsigned long addr_dest = ((regs.es*16)+regs.bx.bx) + (unsigned long)virtual_memory_base_address;

          if(args.devices->disks[drive_number]) {
            unsigned short* addr_disk = (unsigned short*)(args.devices->disks[drive_number]->addr);
            addr_disk+=(cyl_number * DISK_HEADS_PER_CYL + head_number)*DISK_SECTORS_PER_CYL + sector_base;
            
            // TODO 2 bytes por vez, mas podemos fazer 8 bytes por vez pra economizar ciclos
            
            std::cout << "SECTORS TO READ: " << sectors_to_read << "\n";
            std::cout << "VIRTUAL MEMORY ADDRESS TO WRITE: " << (unsigned long)((unsigned short*)addr_dest)-(unsigned long)virtual_memory_base_address;
            
            for(int sector = 0; sector < sectors_to_read ; sector++) {
              std::cout << "SECTOR: " << sector << "\n";
              for(int _byte = 0; _byte<256; _byte++) {

                int ACTUAL_SECTOR = sector+sector_base;
                unsigned short* actual_addr_buffer = addr_disk+ACTUAL_SECTOR*256+_byte;

                *((unsigned short*)addr_dest+ACTUAL_SECTOR*256+_byte) = *actual_addr_buffer;
                
                std::cout << "write to byte " << _byte*2 << " at " << (unsigned long)((unsigned short*)addr_dest+ACTUAL_SECTOR*256+_byte) << "\t";
                std::cout << "content: 0x" << itoh(*actual_addr_buffer) << "\n";
              }

            }
          } else {
            CF = 1;
          }
          regs.pc+=2;
          break;
        };

      case 0x16:
        {
          if(regs.ax.ah == 0) {
            while(!CPU.keyboard_pendent_interrupt) {}
            regs.ax.al = CPU.last_key;
            // TODO colocar o scancode no AH
            CPU.keyboard_pendent_interrupt = false;
            // TODO remover a interrupção da fila também
            // TODO modificar o retorno dependendo do estado das teclas de controle
            regs.pc+=2;
            break;
          } else if(regs.ax.ah == 1) {
            if(CPU.keyboard_pendent_interrupt) {
              ZF = 0;
              regs.ax.al = CPU.last_key;
              // TODO scancode em AH
            } else {
              ZF = 1;
            }
            break;
            // TODO será q o int da BIOS pro teclado tira o CPU.keyboard_pendent_interrupt? tira a key da fila tbm?
          } else if(regs.ax.ah == 2) {
            // TODO checar estado das teclas SHIFT, CTRL, ALT
          } else if(regs.ax.ah == 3) {
            // TODO ?
          } else if(regs.ax.ah == 4) {/*TODO*/}
          else if(regs.ax.ah == 5){
            CPU.last_key = regs.cx.cl;
            //int_queue.push({KEYBOARD, new KeyboardInterruption(CPU.last_key)}); TODO isso é realmente necessário?
            break;
          }
        };
      
      default:
        {
          std::cout << "[DBG] Warning: Interrupt Number " << args.imm8_value << " not implemented yet\n";
          regs.pc += 2;
          break;
        };
    }
  }

  void NotImplemented(DEFAULT_ARGS) {
    unsigned char size = opcode_map[regs.ir].size;
    regs.pc+=size;
    return;
  }

  unsigned char* bits8_find_reg_by_index(unsigned char index) {
    switch(index) {
      case 0: return &(regs.ax.al);
      case 1: return &(regs.cx.cl);
      case 2: return &(regs.dx.dl);
      case 3: return &(regs.bx.bl);
      case 4: return &(regs.ax.ah);
      case 5: return &(regs.cx.ch);
      case 6: return &(regs.dx.dh);
      case 7: return &(regs.bx.bh);
      default: return &(regs.ax.al); // TODO cpu fault invalid opcode
    }
    return &regs.ax.al; // TODO FIXME temporary
  }

  void _JMP_short(DEFAULT_ARGS) {
    signed char offset = (signed char) (args.imm8_value & 0xFF);
    jump_to(offset+2);
  }

  unsigned short calculate_effective_address(unsigned char Mod, unsigned char R_M, unsigned int displacement) {
    unsigned int effective_address = 0;
    return 0;
#if 0 
    switch (Mod) {
        case 0b00:
            // Endereçamento indireto, exemplo: [BX]
            if (R_M == 0b111) {
                effective_address = displacement;
            } else if (R_M == 0b011) {
                effective_address = *(regs.bx) + *DI;
            }
            // Aqui você trataria outros casos para R_M
            break;
        case 0b01:
            // Endereçamento base + deslocamento de 8 bits
            if (R_M == 0b011) {
                effective_address = *BX + *DI + (char)displacement;  // casting para signed 8-bit
            }
            // Outros casos para R_M
            break;
        case 0b10:
            // Endereçamento base + deslocamento de 16 bits
            if (R_M == 0b011) {
                effective_address = *BX + *DI + displacement;
            }
            // Outros casos para R_M
            break;
        case 0b11:
            // Endereçamento de registro, não é necessário calcular um endereço efetivo aqui
            break;
    }
    return effective_address;
#endif
  }

  void _ADD_regoraddr_8bits(DEFAULT_ARGS) {
    unsigned char byte_modrm = args.imm8_value;
    std::cout << "args.imm8_value: " << (unsigned short)args.imm8_value << std::endl;

    unsigned char first_operand_is_reg_or_memoryref = (byte_modrm) & 0xC0; // MOD
    unsigned char reg_or_opcode_extension = ((byte_modrm) & 0x38) >> 3; // DEST REG
    unsigned char second_operand_reg_or_addrmode = (byte_modrm) & 0x07; // ORIG REG 
    
    unsigned char Mod = first_operand_is_reg_or_memoryref >> 6;
    unsigned char Reg_or_Opcode = reg_or_opcode_extension;
    unsigned char R_M = second_operand_reg_or_addrmode;
    std::cout << "Mod: " << (unsigned short)Mod << std::endl;
    if(Mod == 0b11) {
      // TWO 8-bit REGISTERS
      std::cout << "add reg, reg\n";
      unsigned char dest_reg_index = (reg_or_opcode_extension);
      unsigned char src_reg_index = second_operand_reg_or_addrmode & 0x07;

      std::cout << "dest_reg_index: " << (unsigned short)dest_reg_index << " src_reg_index: " << (unsigned short) src_reg_index << std::endl;

      unsigned char* dest_reg = bits8_find_reg_by_index(dest_reg_index);
      unsigned char* src_reg = bits8_find_reg_by_index(src_reg_index);

      *(dest_reg) += *(src_reg);
      
      unsigned char result = *(dest_reg);

      __set_flags(result);
      regs.pc+=2;
      return;
      // TODO check OF

    } else {
      // ex: mov [di], di TODO!
      unsigned short effective_address = calculate_effective_address(Mod, R_M /*offset*/, 0);
      unsigned char* src_reg = bits8_find_reg_by_index(reg_or_opcode_extension);
      unsigned char dest_value;

      if(Mod != 0b00) {
        dest_value = __read_byte(effective_address, 0);
      } else {
        // TODO
        dest_value = __read_byte(effective_address, 0);
      }

      dest_value += *src_reg;

      *(virtual_memory_base_address+effective_address) = dest_value;

      __set_flags(dest_value);
      

      regs.pc += 2;
      return;
      // TODO check OF
    }


  }

  void _ADD_regoraddr_16bits(DEFAULT_ARGS) {
    std::cout << "add regoraddr 16 bits not implemented yet\n";
    while(true);
  }

  void _IN_al_dx(DEFAULT_ARGS) {
    // TODO FIXME implementar
    regs.ax.al = 1;
  }

  namespace MOV {

    void _RM16_R16(DEFAULT_ARGS) { // 0x89
      // HARD CODED
      unsigned char ModR_M = args.imm16_value & 0x00FF;
      unsigned char Mod = (ModR_M & 0xC0) >> 6;
      unsigned char Reg_Opcode = (ModR_M & 0x38) >> 3;
      unsigned char RM = ModR_M & 0x07;
      
      unsigned short* rhs = get_register_by_index(Reg_Opcode);
      unsigned short* lhs = get_register_by_index(RM);

      std::cout << "mod: " << (unsigned short)Mod << std::endl;
      
      if(Mod == 0b11) { // ex: mov di, ax
        *lhs = *rhs;
        regs.pc+=2;
        return;
      } else if (Mod == 0b00) { // ex: mov [di], ax
        // TODO registrador BP e SP usa o segmento padrão SS, mas n to colocando isso agora
        // to usando o DS neles também
        unsigned short* addr = (unsigned short*) (virtual_memory_base_address+regs.ds*16+(*lhs));
        *addr = *rhs;
        std::cout << "addr: " << (unsigned long)addr;
        regs.pc += 2;
        return;
      } else {
        std::cout << "Mod nao implementado ainda no opcode 0x89";
        while(true);
      }

    }

    void _R8_RM8(DEFAULT_ARGS) {
      // HARD CODED
      //regs.ax.al = __read_byte(regs.si, regs.ds*16);
      regs.ax.al = *(((unsigned char*)virtual_memory_base_address)+(regs.ds*16)+regs.si);
      regs.pc+=2;
    }

    void _AX_imm16(DEFAULT_ARGS) { // 0xB8
      regs.ax.ax = args.imm16_value;
      regs.pc += 3;
    }

    void _SI_imm16(DEFAULT_ARGS) { // 0xBE
      std::cout << "MOV si, " << args.imm16_value << "\n";
      regs.si = args.imm16_value;
      regs.pc += 3;
    }

    void _DI_imm16(DEFAULT_ARGS) { // 0xBF
      regs.di = args.imm16_value;
      regs.pc += 3;
    }

    void _SP_imm16(DEFAULT_ARGS) { // 0xBC
      regs.sp = args.imm16_value;
      regs.pc += 3;
    }

    void _AL_imm8(DEFAULT_ARGS) {
      //regs.ax = (regs.ax&AH) | args.imm8_value;
      regs.ax.al = args.imm8_value;
      regs.pc += 2;
    }

    void _BH_imm8(DEFAULT_ARGS) { // 0xB7
      regs.bx.bl = args.imm8_value;
      regs.pc+=2;
    }

    void _DH_imm8(DEFAULT_ARGS) { // 0xB6
      regs.dx.dh = args.imm8_value;
      regs.pc += 2;
    }

    void _CH_imm8(DEFAULT_ARGS) { // 0xB5
      regs.cx.ch = args.imm8_value;
      regs.pc += 2;
    }

    void _DL_imm8(DEFAULT_ARGS) { // 0xB2
      regs.dx.dl = args.imm8_value;
      regs.pc += 2;
    }

    void _BX_imm16(DEFAULT_ARGS) { // 0xBB
      regs.bx.bx = args.imm16_value;
      regs.pc += 3;
    }

    void _AH_imm8(DEFAULT_ARGS) { // 0xB4
      regs.ax.ah = args.imm8_value;
      regs.pc += 2;
    }

    void _CL_imm8(DEFAULT_ARGS) {
      regs.cx.cl = args.imm8_value;
      regs.pc += 2;
    }

  }

  namespace CALL {
    void _rel16(DEFAULT_ARGS) {
      signed short offset = args.imm16_value;
      std::cout << "offset: " << offset << "\n";
      regs.sp -= 2;
      // Salva o return address
      unsigned short* sp_ptr = (unsigned short*)(virtual_memory_base_address+(regs.ss*16)+regs.sp);
      *sp_ptr = regs.pc+3;
      // ---
      regs.pc += offset+3;
      // TODO FIXME fazer a parada da stack e tal
    }
  }

  namespace CMP {
    void _al_imm8(DEFAULT_ARGS) { // 0x3C
      signed char value_to_compare = args.imm8_value;
      signed char result = regs.ax.al - value_to_compare;
      // TODO acho que nem todas flags estão corretas, algumas devem estar faltando
      if(result == 0) {
        std::cout << "CMP resultou em TRUE (valor ficou em zero)\n";
      }
      __set_flags(result);
      //else if (result < 0) TODO!
      regs.pc += 2;

    }
  
  }

  void _JMP_if_equals(DEFAULT_ARGS) {
    if(ZF == 1) {
      regs.pc += (signed char)args.imm8_value;
      std::cout << "JUMP IF EQUALS: IS EQUALS!\n";
    }
    regs.pc += 2;
  }

}
