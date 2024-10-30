#include <stdint.h>
/*
1 shifted left by 16 bits,
binary number 10000000000000000, which equals 65,536 in decimal
*/
#define MEMORY_MAX (1 << 16)
uint16_t memory[MEMORY_MAX]; //65536 locations

/*Registers*/
enum
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, /* program counter */
    R_COND,
    R_COUNT
};

uint16_t reg[R_COUNT]; // store the registers in an array

// why we use enum--> automatic incrementing, Sequential Representation
enum
{
    FL_POS = 1 << 0, /* FL_POS is set to 1 shifted left by 0 bits.
                         In binary (16-bit): 0000 0000 0000 0001
                         Decimal value: 1
                         Represents the "positive" state flag. */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};


/*Instruction Set*/
enum
{
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

int main(int argc, const char* argv[])
{
    
    reg[R_COND] = FL_ZRO;

    /* Set the PC to the starting memory address.
       0x3000 is typically the default start address in this context. */
    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    int running = 1;
    while (running)
    {
        /* FETCH */
        /* Read the instruction at the address in the PC register,
           then increment the PC to point to the next instruction. */
        uint16_t instr = mem_read(reg[R_PC]++);
        
        /* DECODE */
        /* The opcode is determined by shifting the instruction 12 bits to the right.
           The opcode is stored in the upper 4 bits of the 16-bit instruction. */
        uint16_t op = instr >> 12;

        /* EXECUTE */
        switch (op)
        {
            case OP_ADD:
                /* @{ADD} */
                break;
            case OP_AND:
                /* @{AND} */
                break;
            case OP_NOT:
                /* @{NOT} */
                break;
            case OP_BR:
                /* @{BR} */
                break;
            case OP_JMP:
                /* @{JMP} */
                break;
            case OP_JSR:
                /* @{JSR} */
                break;
            case OP_LD:
                /* @{LD} */
                break;
            case OP_LDI:
                /* @{LDI} */
                break;
            case OP_LDR:
                /* @{LDR} */
                break;
            case OP_LEA:
                /* @{LEA} */
                break;
            case OP_ST:
                /* @{ST}. */
                break;
            case OP_STI:
                /* @{STI} */
                break;
            case OP_STR:
                /* @{STR} */
                break;
            case OP_TRAP:
                /* @{TRAP} */
                break;
            case OP_RES:
            case OP_RTI:
            default:
                /* @{BAD OPCODE} */
                break;
        }
    }
    
    /* @{Shutdown} */
}
