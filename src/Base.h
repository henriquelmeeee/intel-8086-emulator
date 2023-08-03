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

/* FLAGS
 bits
      0->CF->Carry Flag
      1->Reserved
      2->PF->Parity Flag
      3->Reserved
      4->AF->Auxiliary Carry Flag
      5->Reserved
      6->Zero Flag
      7->Sign Flag
      8->Trap Flag (debugging purposes)
      9->IF->Interrupt Flag
      10->DF->Direction Flag
      11->OF->Overflow Flag
      12,13,14,15->IOPL->I/O Privilege Level Field

      NOW, x86-32BITS FLAGS RELATED, NOT RELEVANT IN THIS CASE!!!!!!!!!!!!!
      16->NT->Nested Task Flag
      17->Reserved
      18->RF->Resume Flag (debugging purposes)
      19->VM->Virtual 8086 mode
      20->AC->Allignment Check
      21->VIF->Virtual Interrupt Flag
      22->VIP->Virtual Interrupt Pending
      23->ID->ID flag
      24,25,26,27,28,29,30,31->Reserved
*/

#define CF ( (regs.flags)&1 )
#define ZF ( (regs.flags)&128 )

enum ExecutionState {
    SUCCESS,
    ERROR,
};

// LITTLE ENDIAN!!!!!!!
#define AH 0xFF00
#define AL 0x00FF

#endif
