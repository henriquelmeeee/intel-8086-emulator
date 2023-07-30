#ifndef BASE
#define BASE

typedef unsigned short word;

struct Registers {
    /* General-Purposes registers */
    word ax;
    word cx;
    word dx;
    word bx;
    word sp;
    word bp;
    word si;
    word di;
    word pc;
    word flags;
    word cs;
    word ss;
    word ds;
    word es;
    unsigned short ir;
} extern regs;

enum ExecutionState {
    SUCCESS,
    ERROR,
};

// LITTLE ENDIAN!!!!!!!
#define AH 0xFF00
#define AL 0x00FF

#endif
