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
    
    // 1-byte opcodes
    regs.ir = std::htons(regs.ir);
    switch ((regs.ir) >> 8) {
        case NOP: {
            ++regs.pc;
            break;
        };
        default: {
            break;
        };
    }
    
    // 2-byte opcodes test
    switch( (regs.ir) >> 8 ) {
        case INT: {
            
            switch( (regs.ir)&0x00FF ) {
                
                case 0x10: { // BIOS VIDEO SERVICE

                    switch( (regs.ax)&AH ) { // AH
                        case 0x0e: { // DISPLAY CARACTER
                            cout << ((regs.ax)&AL);
                            break;
                        };
                        default: {
                            cout << "?";
                        };
                    }
                };
   
                case 0x03: {
                   while(true);
                };
                
            }
            
            regs.pc+=2;
            break;
        };
        default: {
            break;
        };
    }
    
    switch( (regs.ir)>>8 ) {
        case NOP: {
            regs.pc+=1;
            break;
        };
        
        default: {
            cout << "CPU Fault";
            while(true);
        };
    }

    return {};

}

extern "C" void start_execution_by_clock() {
    while(true) {
        //cout << "Execução de RIP em 0x" << itoh(regs.pc) << "\n";
        
        word instruction_offset = (regs.cs*16) + regs.pc;
        regs.ir = *(virtual_memory_base_address+regs.cs+regs.pc);
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
        virtual_memory_base_address = (byte*) mmap(NULL, 512*MB, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        cout << "Endereço base da memória virtual alocada (512MB): 0x" \
                    << itoh((unsigned long)virtual_memory_base_address) << "\n";
                    
        cout << "Carregando bootloader (setor 0) para memória no endereço-offset 0x7c00...\n";
        
        byte *disk = (byte*) std::fopen("source", "rw");
        for(int byte_ = 0; byte_ < 512; byte_++) {
            cout << ".";
            *(virtual_memory_base_address+0x7c00+byte_) = *(disk+byte_);
        }
        cout << "\n";
        
        regs.pc = 0x7c00;
        regs.cs = 0;
        
        start_execution_by_clock();
        
        
        
    }
    
    return 0;
}
