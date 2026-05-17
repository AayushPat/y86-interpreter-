/*
 * CS 261 PA3: Mini-ELF disassembler
 *
 * Name:
 */

#include "p3-disas.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

y86_inst_t fetch (y86_t *cpu, byte_t *memory)
{
    y86_inst_t ins;
    //null checks
    if (cpu == NULL || memory == NULL) {
        ins.icode = INVALID;
        return ins;
    }
    //check the size
    if (cpu->pc >= MEMSIZE) {
        cpu->stat = ADR;
        ins.icode = INVALID;
        return ins;
    }
    //mask the first and last 4 bits for icode and ifun
    ins.icode = (memory[cpu->pc] >> 4) & 0xF;
    ins.ifun.b  = memory[cpu->pc] & 0xF;
    bool need_regids = false;
    //check if instruction needs register
    bool need_valC = false;
    //check if instruction needs a destination in memory

    //switch code to check which instruction is given and mark the correct values accordingly
    switch (ins.icode) {
        case INVALID:
            cpu->stat = INS;
            return ins;
        case HALT:
            if(ins.ifun.b != 0) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            break;
        case NOP:
            if(ins.ifun.b != 0) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            break;
        case CALL:
            if (ins.ifun.b != 0) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            need_valC = true;
            break;
        case RET:
            if(ins.ifun.b != 0) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            break;
        case PUSHQ:
            if (ins.ifun.b != 0) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            need_regids = true;
            break;
        case POPQ:
            if (ins.ifun.b != 0) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            need_regids = true;
            break;
        case IRMOVQ:
            if (ins.ifun.b != 0) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            need_valC = true;
            need_regids = true;
            break;
        case RMMOVQ:
            if (ins.ifun.b != 0) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            need_valC = true;
            need_regids = true;
            break;
        case MRMOVQ:
            if (ins.ifun.b != 0) {
                cpu->stat = INS;
                ins.icode = INVALID;
                return ins;
            }
            need_valC = true;
            need_regids = true;
            break;
        case CMOV:
            if (ins.ifun.b >= BADCMOV) {
                cpu->stat = INS;
                ins.icode = INVALID;
                return ins;
            }
            need_regids = true;
            break;
        case OPQ:
            if (ins.ifun.b >= BADOP) {
                cpu->stat = INS;
                ins.icode = INVALID;
                return ins;
            }
            need_regids = true;
            break;
        case JUMP:
            if (ins.ifun.b >= BADJUMP) {
                cpu->stat = INS;
                ins.icode = INVALID;
                return ins;
            }
            need_valC = true;
            break;
        case IOTRAP:
            if (ins.ifun.b >= BADTRAP) {
                cpu->stat = INS;
                ins.icode = INVALID;
                return ins;
            }
            break;
        default:
            ins.icode = INVALID;
            cpu->stat = INS;
            return ins;
    }
    //offset
    int offset = 1;

    ins.valP = cpu ->pc + 1;

    //checks if the given instructions need registers
    if (need_regids) {
        //make sure we are not out of bounds
        if ((cpu->pc + 1) >= MEMSIZE) {
            cpu->stat = ADR;
            ins.icode = INVALID;
            return ins;
        }
        //reads the regesters then masks the first and last 4 bits into the stuct also set offset pased these bits
        byte_t regbyte = memory[cpu->pc + 1];
        ins.ra = (regbyte >> 4) & 0xF;
        ins.rb = regbyte & 0xF;
        ins.valP +=1;
        offset = 2;
        if (ins.icode == IRMOVQ) {
            if (ins.ra != NOREG || ins.rb == NOREG) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
        } else if (ins.icode == PUSHQ || ins.icode == POPQ) {
            if (ins.ra == NOREG || ins.rb != NOREG) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
        } else if (ins.icode == RMMOVQ || ins.icode == MRMOVQ) {
            if (ins.ra == NOREG) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
        } else {
            if (ins.ra == NOREG || ins.rb == NOREG) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
        }
    }
    // checks if it needs an address and assigns it if yeah
    if (need_valC) {
        if ((cpu->pc + offset + 8) > MEMSIZE) {
            cpu->stat = ADR;
            ins.icode = INVALID;
            return ins;
        }

        ins.valC.v = *((int64_t*)&memory[cpu->pc + offset]);
        ins.valP +=8;
    }
    cpu->stat = AOK;
    return ins;
}


/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void disassemble (y86_inst_t *inst)
{
    //null check
    if (inst == NULL) {
        return;
    }
    //registers
    const char *reg_names[] = {
        "%rax", "%rcx", "%rdx", "%rbx", "%rsp", "%rbp", "%rsi", "%rdi",
        "%r8",  "%r9",  "%r10", "%r11", "%r12", "%r13", "%r14", "NOREG"
    };
    printf("  ");
    //switch case for instructions
    switch (inst->icode) {
        case HALT:
            printf("halt");
            break;
        case NOP:
            printf("nop");
            break;
        case RET:
            printf("ret");
            break;
 
        case CALL:
            printf("call 0x%lx", inst->valC.v);
            break;

        case JUMP:
            // jmp, jle, jl, je, jne, jge, jg
            switch (inst->ifun.b) {
                case JMP:
                    printf("jmp");
                    break;
                case JLE:
                    printf("jle");
                    break;
                case JL:
                    printf("jl");
                    break;
                case JE:
                    printf("je");
                    break;
                case JNE:
                    printf("jne");
                    break;
                case JGE:
                    printf("jge");
                    break;
                case JG:
                    printf("jg");
                    break;
            }
            printf(" 0x%lx", inst->valC.v);
            break;

        case CMOV:
            //RRMOVQ, CMOVLE, CMOVL, CMOVE, CMOVNE, CMOVGE, CMOVG,
            switch (inst->ifun.b) {
                case RRMOVQ:
                    printf("rrmovq");
                    break;
                case CMOVLE:
                    printf("cmovle");
                    break;
                case CMOVL:
                    printf("cmovl");
                    break;
                case CMOVE:
                    printf("cmove");
                    break;
                case CMOVNE:
                    printf("cmovne");
                    break;
                case CMOVGE:
                    printf("cmovge");
                    break;
                case CMOVG:
                    printf("cmovg");
                    break;
            }
            printf(" %s, %s", reg_names[inst->ra], reg_names[inst->rb]);
            break;

        case OPQ:
            switch (inst->ifun.b) {
                //
                case ADD:
                    printf("addq");
                    break;
                case SUB:
                    printf("subq");
                    break;
                case AND:
                    printf("andq");
                    break;
                case XOR:
                    printf("xorq");
                    break;
            }
            printf(" %s, %s", reg_names[inst->ra], reg_names[inst->rb]);
            break;

        case IRMOVQ:
            printf("irmovq 0x%lx, %s", inst->valC.v, reg_names[inst->rb]);
            break;

        case RMMOVQ:
            printf("rmmovq %s, 0x%lx", reg_names[inst->ra], inst->valC.v);
            if (inst->rb != NOREG) {
                printf("(%s)", reg_names[inst->rb]);
            }
            break;

        case MRMOVQ:
            printf("mrmovq 0x%lx", inst->valC.v);
            if (inst->rb != NOREG) {
                printf("(%s)", reg_names[inst->rb]);
            }
            printf(", %s", reg_names[inst->ra]);
            break;

        case PUSHQ:
            printf("pushq %s", reg_names[inst->ra]);
            break;

        case POPQ:
            printf("popq %s", reg_names[inst->ra]);
            break;

        case IOTRAP:
            printf("iotrap %d", inst->ifun.b);
            break;

        case INVALID:
            printf("Failed to read file");
            break;
    }
    //all the print stuff for each instruction
}

void disassemble_code (byte_t *memory, elf_phdr_t *phdr, elf_hdr_t *hdr)
{
    y86_t cpu;
    y86_inst_t inst;
    //print header
    printf("  0x%03x:                               | .pos 0x%03x code\n",
           phdr->p_vaddr, phdr->p_vaddr);

    cpu.pc = phdr->p_vaddr;
    //print all the other information
    while (cpu.pc < phdr->p_vaddr + phdr->p_size) {
        if (cpu.pc == hdr->e_entry) {
            printf("  0x%03lx:                               | _start:\n", cpu.pc);
        }

        uint64_t old_pc = cpu.pc;
        inst = fetch(&cpu, memory);

        if (inst.icode == INVALID) {
            printf("Invalid opcode: 0x%02x\n", memory[old_pc]);
            break;
        }
        if (inst.valP <= old_pc) {
            printf("Failed to read file");
            break;
        }

        printf("  0x%03lx: ", old_pc);

        int byte_count = inst.valP - old_pc;
        for (int i = 0; i < byte_count; i++) {
            printf("%02x ", memory[old_pc + i]);
        }
        for (int i = byte_count; i < 10; i++) {
            printf("   ");
        }
        printf("| ");
        disassemble(&inst);
        printf("\n");

        cpu.pc = inst.valP;
    }
}

void disassemble_data (byte_t *memory, elf_phdr_t *phdr)
{
    //null checks
    if (memory == NULL || phdr == NULL) {
        return;
    }
    printf("  0x%03x:                               | .pos 0x%03x data\n",
           phdr->p_vaddr, phdr->p_vaddr);

    for (int i = 0; i < phdr->p_size; i += 8) {
        uint64_t val = *((uint64_t*)&memory[phdr->p_vaddr + i]);

        // print address
        printf("  0x%03x: ", phdr->p_vaddr + i);

        // print hex bytes
        for (int j = 0; j < 8; j++) {
            printf("%02x ", memory[phdr->p_vaddr + i + j]);
        }

        // padding + directive
        printf("      |   .quad 0x%lx\n", val);
    }
}

void disassemble_rodata (byte_t *memory, elf_phdr_t *phdr)
{
    printf("  0x%03x:                               | .pos 0x%03x rodata\n",
           phdr->p_vaddr, phdr->p_vaddr);

    int i = 0;
    while (i < phdr->p_size) {
        int str_start = i;

        // find end of string (including null terminator)
        int str_end = i;
        while (str_end < phdr->p_size && memory[phdr->p_vaddr + str_end] != '\0') {
            str_end++;
        }
        str_end++; // include null terminator

        bool first_line = true;
        while (i < str_end) {
            int bytes_on_line = 0;

            printf("  0x%03x: ", phdr->p_vaddr + i);

            // print up to 10 bytes
            while (i < str_end && bytes_on_line < 10) {
                printf("%02x ", memory[phdr->p_vaddr + i]);
                bytes_on_line++;
                i++;
            }

            // pad to align |
            for (int j = bytes_on_line; j < 10; j++) {
                printf("   ");
            }

            if (first_line) {
                printf("|   .string \"%s\"\n", &memory[phdr->p_vaddr + str_start]);
                first_line = false;
            } else {
                printf("| \n");
            }
        }
    }
}

