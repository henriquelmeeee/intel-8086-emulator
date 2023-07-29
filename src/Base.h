#ifndef BASE
#define BASE

typedef unsigned short word;

struct Registers {
    /* General-Purposes registers */
    word ax;
    /* Code-related registers */
    word ir;                            // Instruction Register
    word pc;                            // Program Counter 
    /* Segmentation-related registers */
    word cs;                            // Code segment (start address)
} extern regs;

enum ExecutionState {
    SUCCESS,
    ERROR,
};

// LITTLE ENDIAN!!!!!!!
#define AH 0xFF00
#define AL 0x00FF

#endif
