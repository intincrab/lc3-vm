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
            case OP_ADD: {
                /*destination register (DR) */
                uint16_t r0 = (instr >> 9) & 0x7;
                /* first operand (SR1)*/
                // Extract first operand register (SR1) from bits 6-8 of instruction
                uint16_t r1 = (instr >> 6) & 0x7;
                // Extract the immediate flag (1 if using immediate mode, 0 if using register mode)
                uint16_t imm_flag = (instr >> 5) && 0x1;

                if(imm_flag)
                {
                    /* Extract 5-bit immediate value
                    from bits 0-4, sign-extend it to 16 bits */
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[1] + imm5;
                }
                else
                {
                    uint16_t r2 = instr & 0x7;
                    reg[r0] = reg[1] + reg[r2];
                }
                
                update_flags(r0);
            } break;
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
    // ADD R2 R0 R1 ; add the contents of R0 to R1 and store in R2.
    if(argc < 2)
    {
        /* show usage string */
        printf("lc3 [image-file] ...\n");
        exit(2);
    }

    for(int j = 1; j < argc; ++j)
    {
        if(!read_image(argv[j]))
        {
            printf("failed to load the image: %s\n", argv[j]);
            exiit(1);
        }
    }

    uint16_t sign_extend(uint16_t x,int bit_count)
    {
        if((x >> ( bit_count - 1)) & 1){
            x |= (0xFFFF << bit_count);
        }
        return x;
    }

    void update_flags(uint16_t r)
    {
        if(reg[r] == 0)
        {
            reg[R_COND] = FL_ZRO;
        }
        else if(reg[r] >> 15) /* a 1 in the left-most bit indicates negative */
        {
            reg[R_COND] = FL_NEG;
        }
        else
        {
            reg[R_COND] = FL_POS;
        }

    }




    
    /* @{Shutdown} */
}
