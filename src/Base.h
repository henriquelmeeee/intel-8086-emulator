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

#endif
