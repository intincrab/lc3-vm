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

enum
{
    TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25   /* halt the program */
};

enum
{
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02  /* keyboard data */
};

void mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

uint16_t mem_read(uint16_t address)
{
    if (address == MR_KBSR)
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}

uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

int read_image(const char* image_path)
{
    FILE* file = fopen(image_path, "rb");
    if (!file) { return 0; };
    read_image_file(file);
    fclose(file);
    return 1;
}

void read_image_file(FILE* file)
{
    // origin tells us where in memory to place image
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin); //convert from big-endian to little-endian format

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = MEMORY_MAX - origin; //prevents overflow
    uint16_t* p = memory + origin; //p points to the starting ad where image data will load
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* swap to little endian */
    while (read-- > 0)
    {
        *p = swap16(*p);
        ++p;
    }
}

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
            case OP_AND:{
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t imm_flag = (instr >> 5) & 0x1;

                if (imm_flag)
                {
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] & imm5;
                }
                else
                {
                    uint16_t r2 = instr & 0x7;
                    reg[r0] = reg[r1] & reg[r2];
                }
                update_flags(r0);
            }break;
            case OP_NOT:{
                uint16_t r0 = (instr >>9 ) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;

                reg[r0] = ~reg[r1];
                update_flags(r0);
            }break;
            case OP_BR:{
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                uint16_t cond_flag = (instr >> 9) & 0x7;
                if (cond_flag & reg[R_COND])
                {
                    reg[R_PC] += pc_offset;
                }
                } break;
            case OP_JMP:
                // Jump: sets PC to the address in the specified register (also handles RET)
                uint16_t r1 = (instr >> 6) & 0x7;
                reg[R_PC] = reg[r1];
                break;
            case OP_JSR: {
                // Jump to Subroutine: saves PC and jumps to offset or register address
                reg[R_R7] = reg[R_PC];

                uint16_t indir_flag = (instr >> 11) & 0x1;
                if (indir_flag) {
                    uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
                    reg[R_PC] += long_pc_offset;
                } else {
                    uint16_t baseR = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[r1]; /* JSRR */
                }
            } break;
            case OP_LD:
                // Load: loads a value from memory at PC+offset into the specified register
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = mem_read(reg[R_PC] + pc_offset);
                update_flags(r0);
                break;
            case OP_LDI:{
                // Load Indirect: loads a value from memory through an address in memory
                /* Extract the destination register (DR) from bits 9-11 of the instruction */
                uint16_t r0 = (instr >> 9) & 0x7;

                /* PCoffset 9*/
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

                /* add pc_offset to the current PC, look 
                at that memory location to get the final address */
                reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
                update_flags();
            }break;
            case OP_LDR:
            {
                // Load Register: loads a value from memory at the address in a base register plus offset
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = sign_extend(instr & 0x3F, 6);
                reg[r0] = mem_read(reg[r1] + offset);
                update_flags(r0);
            }
            break;
            case OP_LEA:
            {
                // Load Effective Address: loads PC+offset into the specified register
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = reg[R_PC] + pc_offset;
                update_flags(r0);
            }
            break;
            case OP_ST:
            {
                // Store: stores the value from a register into memory at PC+offset
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                mem_write(reg[R_PC] + pc_offset, reg[r0]);
            }
            break;
            case OP_STI:
            {
                // Store Indirect: stores the value from a register into an address in memory through another address
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
            }
            break;
            case OP_STR:
            {
                // Store Register: stores the value from a register into memory at base register address plus offset
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = sign_extend(instr & 0x3F, 6);
                mem_write(reg[r1] + offset, reg[r0]);
            }
            break;
            case OP_TRAP:
                reg[R_R7] = reg[R_PC];
                switch (instr & 0xFF)
                {
                case TRAP_GETC:
                    /* read a single ASCII char */
                    reg[R_R0] = (uint16_t)getchar();
                    update_flags(R_R0);
                    break;
                case TRAP_OUT:
                    putc((char)reg[R_R0], stdout);
                    fflush(stdout);
                    break;
                case TRAP_PUTS:
                {
                    /* one char per word */
                    uint16_t *c = memory + reg[R_R0];
                    while (*c)
                    {
                        putc((char)*c, stdout);
                        ++c;
                    }
                    fflush(stdout);
                }
                break;
                case TRAP_IN:
                {
                    printf("Enter a character: ");
                    char c = getchar();
                    putc(c, stdout);
                    fflush(stdout);
                    reg[R_R0] = (uint16_t)c;
                    update_flags(R_R0);
                }
                break;
                case TRAP_PUTSP:
                {
                    /* one char per byte (two bytes per word)
                       here we need to swap back to
                       big endian format */
                    uint16_t *c = memory + reg[R_R0];
                    while (*c)
                    {
                        char char1 = (*c) & 0xFF;
                        putc(char1, stdout);
                        char char2 = (*c) >> 8;
                        if (char2)
                            putc(char2, stdout);
                        ++c;
                    }
                    fflush(stdout);
                }
                break;
                case TRAP_HALT:
                    puts("HALT");
                    fflush(stdout);
                    running = 0;
                    break;
                }
                break;
            case OP_RES:
            case OP_RTI:
            default:
                abort();
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
