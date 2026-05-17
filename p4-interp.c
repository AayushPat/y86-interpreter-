/*
 * CS 261 PA4: Mini-ELF interpreter
 *
 * Name:
 */

#include "p4-interp.h"
static char output_buffer[100] = {0};
static int buf_pos = 0;
//global buffer idk if im supposed to do it like this but the only other way i couold think was pass it as parameter but that seems more wrong

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

y86_reg_t decode_execute (y86_t *cpu, y86_inst_t *inst, bool *cnd, y86_reg_t *valA)
{
    if(cpu == NULL) {
        return 0;   
    }
    //null checks
    y86_reg_t valE = 0;
    if (inst == NULL || cnd == NULL || valA == NULL) {
        cpu->stat = INS;
        return valE;
    }
    //switch case for instructions based on reference sheet 
    switch (inst->icode) {
        case HALT:
            cpu->stat = HLT;
            break;
        case NOP:
            break;
        case RET:
            *valA = cpu->reg[RSP];  // read return address from stack top
            valE = cpu->reg[RSP] + 8;
            break;
        case CALL:
            valE = cpu->reg[RSP] - 8;
            break;
        case JUMP:
            // jmp, jle, jl, je, jne, jge, jg
            switch (inst->ifun.jump) {
                case JMP:
                    *cnd = 1;
                    break;
                case JLE:
                    if (cpu->sf != cpu->of || cpu->zf) {
                        *cnd = 1;
                    } else {
                        *cnd = 0;
                    }
                    break;
                case JL:
                    if (cpu->sf != cpu->of) {
                        *cnd = 1;
                    } else {
                        *cnd = 0;
                    }
                    break;
                case JE:
                    if (cpu->zf) {
                        *cnd = 1;
                    } else {
                        *cnd = 0;
                    }
                    break;
                case JNE:
                    if (!cpu->zf) {
                        *cnd = 1;
                    } else {
                        *cnd = 0;
                    }
                    break;
                case JGE:
                    if (cpu->sf == cpu->of) {
                        *cnd = 1;
                    } else {
                        *cnd = 0;
                    }
                    break;
                case JG:
                    if (!cpu->zf && cpu->sf == cpu->of) {
                        *cnd = 1;
                    } else {
                        *cnd = 0;
                    }
                    break;
                default:
                    cpu->stat = INS;
                    break;
            }
            break;

        case CMOV:
            //RRMOVQ, CMOVLE, CMOVL, CMOVE, CMOVNE, CMOVGE, CMOVG,
            *valA = cpu->reg[inst->ra];
            valE = *valA;
            switch (inst->ifun.cmov) {
                case RRMOVQ:
                    *cnd = 1;
                    break;
                case CMOVLE:
                    if (cpu->sf != cpu->of || cpu->zf) {
                        *cnd = 1;
                    } else {
                        *cnd = 0;
                    }
                    break;
                case CMOVL:
                    if (cpu->sf != cpu->of) {
                        *cnd = 1;
                    } else {
                        *cnd = 0;
                    }
                    break;
                case CMOVE:
                    if (cpu->zf) {
                        *cnd = 1;
                    } else {
                        *cnd = 0;
                    }
                    break;
                case CMOVNE:
                    if (!cpu->zf) {
                        *cnd = 1;
                    } else {
                        *cnd = 0;
                    }
                    break;
                case CMOVGE:
                    if (cpu->sf == cpu->of) {
                        *cnd = 1;
                    } else {
                        *cnd = 0;
                    }
                    break;
                case CMOVG:
                    if (!cpu->zf && cpu->sf == cpu->of) {
                        *cnd = 1;
                    } else {
                        *cnd = 0;
                    }
                    break;
                default:
                    *cnd = 0;
                    cpu->stat = INS;
                    break;
            }
            break;

        case OPQ:
            *valA = cpu->reg[inst->ra];   // first operand
            y86_reg_t valB = cpu->reg[inst->rb];  // second operand
            //may move this to the top idk if this is the right way to be doing this
            switch (inst->ifun.op) {
                case ADD:
                    valE = *valA + valB;
                    cpu->of = ((int64_t)*valA > 0 && (int64_t)valB > 0 && (int64_t)valE < 0) ||
                            ((int64_t)*valA < 0 && (int64_t)valB < 0 && (int64_t)valE >= 0);
                    break;
                case SUB:
                    valE = valB - *valA;
                    cpu->of = ((int64_t)*valA < 0 && (int64_t)valB > 0 && (int64_t)valE < 0) ||
                            ((int64_t)*valA > 0 && (int64_t)valB < 0 && (int64_t)valE >= 0);
                    break;
                case AND:
                    valE = valB & *valA;
                    cpu->of = 0;
                    break;
                case XOR:
                    valE = valB ^ *valA;
                    cpu->of = 0;
                    break;
                default:
                    cpu->stat = INS;
                    return 0;
            }
            cpu->zf = (valE == 0);
            cpu->sf = ((int64_t)valE < 0);
            break;
        case IRMOVQ:
            *valA = cpu->reg[inst->ra];
            valE = inst->valC.v;
            break;

        case RMMOVQ:
            *valA = cpu->reg[inst->ra];
            valE = inst->valC.d + cpu->reg[inst->rb];
            break;

        case MRMOVQ:
            valE = inst->valC.d + cpu->reg[inst->rb];
            break;

        case PUSHQ:
            *valA = cpu->reg[inst->ra];
            valE = cpu->reg[RSP] - 8;
            break;

        case POPQ:
            *valA = cpu->reg[RSP];
            valE = cpu->reg[RSP] + 8;
            break;

        case IOTRAP:
            cpu->pc = inst->valP;
            break;

        case INVALID:
            cpu->stat = INS;
            break;
    }

    return valE;
}

void memory_wb_pc (y86_t *cpu, y86_inst_t *inst, byte_t *memory,
                   bool cnd, y86_reg_t valA, y86_reg_t valE)
{
    if(cpu == NULL) {
        return;
    }
    if (inst == NULL || memory == NULL) {
        cpu->stat = INS;
        return;
    }
    switch (inst->icode) {
        case HALT:
            cpu->pc = inst->valP;
            break;
        case NOP:
            cpu->pc = inst->valP;
            break;
        case RET:
            //not sure if its inst->rb or cpu->rsp
            cpu->pc = *(int64_t*)&memory[valA];  // read 8 bytes from stack
            cpu->reg[RSP] = valE;
            break;
        case CALL:
            if (valE >= MEMSIZE || valE + 8 > MEMSIZE) {
                cpu->stat = ADR;
                break;
            }
            *(int64_t*)&memory[valE] = inst->valP;
            cpu->reg[RSP] = valE;
            cpu->pc = inst->valC.dest;
            break;
        case JUMP:
            if(cnd) {
                cpu->pc = inst->valC.dest;
            } else {
                cpu->pc = inst->valP;
            }
            break;

        case CMOV:
            if(inst->ifun.cmov == BADCMOV) {
                cpu->stat = ADR;
                break;
            }
            if (cnd) {
                cpu->reg[inst->rb] = valE;
            }
            cpu->pc = inst->valP;
            break;

        case OPQ:
            cpu->reg[inst->rb] = valE;
            cpu->pc = inst->valP;
            break;

        case IRMOVQ:
            cpu->reg[inst->rb] = valE;
            cpu->pc = inst->valP;
            break;

        case RMMOVQ:
            *(int64_t*)&memory[valE] = valA;
            cpu->pc = inst->valP;
            break;

        case MRMOVQ:
            if(valE >= MEMSIZE) {
                cpu->stat = ADR;
                break;
            }
            cpu->reg[inst->ra] = *(int64_t*)&memory[valE];
            cpu->pc = inst->valP;
            break;

        case PUSHQ:
            if (valE >= MEMSIZE || valE + 8 > MEMSIZE) {
                cpu->stat = ADR;
                break;
            }
            *(int64_t*)&memory[valE] = valA;
            cpu->reg[RSP] = valE;
            cpu->pc = inst->valP;
            break;
        case POPQ:
        //check if valE is a valid address before trying to read from it same for push and call 
            if (valE >= MEMSIZE || valE + 8 > MEMSIZE) {
                cpu->stat = ADR;
                break;
            }
            cpu->reg[inst->ra] = *(int64_t*)&memory[valA];
            cpu->reg[RSP] = valE;
            cpu->pc = inst->valP;
            break;
        case IOTRAP: {
            switch (inst->ifun.trap) {
                case CHAROUT:
                    // write single byte from memory[%rsi] to output buffer
                    if (buf_pos < 99) {
                        output_buffer[buf_pos++] = (char)memory[cpu->reg[RSI]];
                        output_buffer[buf_pos] = '\0';
                    }
                    break;

                case CHARIN: {
                    // read single character from stdin into memory[%rdi]
                    int c = fgetc(stdin);
                    if (c == EOF) {
                        printf("I/O Error\n");
                        cpu->stat = HLT;
                        break;
                    }
                    memory[cpu->reg[RDI]] = (byte_t)c;
                    break;
                }

                case DECOUT: {
                    // write 64-bit integer from memory[%rsi] as decimal text to output buffer
                    int64_t val = *(int64_t*)&memory[cpu->reg[RSI]];
                    int written = snprintf(output_buffer + buf_pos, 100 - buf_pos, "%ld", val);
                    if (written < 0) {
                        printf("I/O Error\n");
                        cpu->stat = HLT;
                        break;
                    }
                    buf_pos += written;
                    break;
                }

                case DECIN: {
                    // read decimal integer from stdin and store as 64-bit int into memory[%rdi]
                    int64_t val;
                    if (scanf("%ld", &val) != 1) {
                        printf("I/O Error\n");
                        cpu->stat = HLT;
                        break;
                    }
                    *(int64_t*)&memory[cpu->reg[RDI]] = val;
                    break;
                }

                case STROUT: {
                    // write null-terminated string from memory[%rsi] to output buffer
                    char *src = (char*)&memory[cpu->reg[RSI]];
                    int written = snprintf(output_buffer + buf_pos, 100 - buf_pos, "%s", src);
                    if (written < 0) {
                        printf("I/O Error\n");
                        cpu->stat = HLT;
                        break;
                    }
                    buf_pos += written;
                    break;
                }

                case FLUSH:
                    // flush output buffer to stdout and reset buffer
                    printf("%s", output_buffer);
                    fflush(stdout);
                    output_buffer[0] = '\0';
                    buf_pos = 0;
                    break;

                default:
                    // unknown trap ID - I/O error
                    printf("I/O Error\n");
                    cpu->stat = HLT;
                    break;
            }
            // advance PC regardless of trap result
            cpu->pc = inst->valP;
            break;
        }

        case INVALID:
            // invalid instruction - set status and print error
            cpu->stat = INS;
            printf("Failed to read file");
            break;
    }

}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void dump_cpu_state (y86_t *cpu)
{
    //prints the cpu state for integration tests 
    printf("Y86 CPU state:\n");
    printf("    PC: %016lx   flags: Z%d S%d O%d     %s\n", 
        cpu->pc, cpu->zf, cpu->sf, cpu->of, cpu->stat == AOK ? "AOK" : (cpu->stat == HLT ? "HLT" : (cpu->stat == ADR ? "ADR" : "INS")));
    printf("  %%rax: %016lx    %%rcx: %016lx\n", cpu->reg[0], cpu->reg[1]);
    printf("  %%rdx: %016lx    %%rbx: %016lx\n", cpu->reg[2], cpu->reg[3]);
    printf("  %%rsp: %016lx    %%rbp: %016lx\n", cpu->reg[4], cpu->reg[5]);
    printf("  %%rsi: %016lx    %%rdi: %016lx\n", cpu->reg[6], cpu->reg[7]);
    printf("   %%r8: %016lx     %%r9: %016lx\n", cpu->reg[8], cpu->reg[9]);
    printf("  %%r10: %016lx    %%r11: %016lx\n", cpu->reg[10], cpu->reg[11]);
    printf("  %%r12: %016lx    %%r13: %016lx\n", cpu->reg[12], cpu->reg[13]);
    printf("  %%r14: %016lx\n", cpu->reg[14]);    
    }


