#ifndef BASE
#define BASE

struct Registers {
    /* General-Purposes registers */
    word ax;
    /* Code-related registers */
    word ir;                            // Instruction Register
    word pc;                            // Program Counter 
    /* Segmentation-related registers */
    word cs;                            // Code segment (start address)
} regs;

enum ExecutionState {
    SUCCESS,
    ERROR,
};

    
#define AH 0x00FF
#define AL 0xFF00

#endif
