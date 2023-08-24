#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <map>

#include "Instructions.h"
#include "Base.h"
#include "Utils.h"

namespace InstructionHandler {
  void _NOP(DEFAULT_ARGS) {
    regs.pc+=1;
    return;
  }

  void _HLT(DEFAULT_ARGS) {
    CPU.hlt = true;
    regs.pc+=1;
    return;
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
    return &regs.ax.al; // TODO FIXME temporary
  }

  void _JMP_short(DEFAULT_ARGS) {
    signed char offset = (signed char) (args.imm8_value & 0xFF);
    jump_to(offset+2);
  }

  void _ADD_regoraddr_8bits(DEFAULT_ARGS) {
    unsigned char byte_modrm = args.imm8_value;

    unsigned char first_operand_is_reg_or_memoryref = (byte_modrm) & 0xC0; // MOD
    unsigned char reg_or_opcode_extension = ((byte_modrm) & 0x38) >> 3; // DEST REG
    unsigned char second_operand_reg_or_addrmode = (byte_modrm) & 0x07; // ORIG REG 
    
    unsigned char Mod = first_operand_is_reg_or_memoryref;
    unsigned char Reg_or_Opcode = reg_or_opcode_extension;
    unsigned char R_M = second_operand_reg_or_addrmode;

    if(Mod == 0b00000011) {
      // TWO 8-bit REGISTERS
      unsigned char dest_reg_index = (reg_or_opcode_extension >> 3) & 0x07;
      unsigned char src_reg_index = second_operand_reg_or_addrmode & 0x07;

      unsigned char* dest_reg = bits8_find_reg_by_index(dest_reg_index);
      unsigned char* src_reg = bits8_find_reg_by_index(src_reg_index);

      *(dest_reg) += *(src_reg);
      
      unsigned char result = *(dest_reg);

      if(result == 0)
        ZF = 1;
      // TODO continue this


    } else {
      std::cout << "MOD not implemented yet\n";
    }


    regs.pc += 3;
  }

  void _IN_al_dx(DEFAULT_ARGS) {
    // TODO FIXME implementar
    regs.ax.al = 1;
  }

  namespace MOV {

    void _AX_imm16(DEFAULT_ARGS) { // 0xB8
      regs.ax.ax = args.imm16_value;
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
      std::cout << "called CALL::_rel16\n";
      regs.pc+=args.imm16_value+2;
      // TODO FIXME fazer a parada da stack e tal
    }
  }
}
