/*
 * CS 261: Main driver
 *
 * Name:
 */

#include "p1-check.h"
#include "p2-load.h"
#include "p3-disas.h"
#include "p4-interp.h"

/*
 * helper function for printing help text
 */
void usage (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");
    printf("  -d      Disassemble code contents\n");
    printf("  -D      Disassemble data contents\n");
    printf("  -e      Execute program\n");
    printf("  -E      Execute program (trace mode)\n");
}

int main (int argc, char **argv)
{
    int opt;
    bool showheader = false;
    bool showphdrs = false;
    bool loadmem = false;
    bool showmembrief = false;
    bool showmemfull = false;
    bool showhelp = false;
    bool disCode = false;
    bool disDat = false;
    bool execute = false;
    bool trace = false;
    //bunch of bools to help execute the right functions
    while ((opt = getopt(argc, argv, "hHafsmMdDeE")) != -1) {
        switch(opt) {
            case 'h':
                // Display usage
                showhelp = true;
            case 'H':
                //Show the Mini-ELF header
                showheader = true;
                break;
            case 'a':
                //Show all with brief memory
                showheader = true;
                showphdrs = true;
                showmembrief = true;
                loadmem = true;
                break;
            case 'f':
                //Show all with full memory
                showheader = true;
                showphdrs = true;
                showmemfull = true;
                loadmem = true;
                break;
            case 's':
                //Show the program headers
                showphdrs = true;
                break;
            case 'm':
                //Show the memory contents (brief)
                showmembrief = true;
                loadmem = true;
                break;
            case 'M':
                //Show the memory contents (full)
                showmemfull = true;
                loadmem = true;
                break;
            case 'd':
                disCode = true;
                loadmem = true;
                break;
            case 'D':
                disDat = true;
                loadmem = true;
                break;
            case 'e':
                execute= true;
                loadmem = true;
                break;
            case 'E':
                trace = true;
                showmemfull = true;
                loadmem = true;
                break;
            default:
                return EXIT_FAILURE;
        }
    }
    
//check for invalid combinations of options and print usage if there are any
    if ((showhelp == true) || (showmembrief == true && showmemfull == true || (execute && trace))) {
        usage(argv);
        return EXIT_FAILURE;
    }
    //check for more args/multi files
    if (argc - optind != 1) {
        usage(argv);
        return EXIT_FAILURE;
    }

    //open file
    FILE *f = fopen(argv[optind], "rb");
    if (f == NULL) {
        printf("Failed to read file\n");
        return EXIT_FAILURE;
    }

    elf_hdr_t hdr;

    if (read_header(f, &hdr)) {
        //reads the header, if it fails it will return false and exit with failure
    } else {
        printf("Failed to read file\n");
        return EXIT_FAILURE;
    }

    if (showheader) {
        dump_header(&hdr);
        //prints all the header details
    }
    elf_phdr_t phdrs[hdr.e_num_phdr];
    for (int i = 0; i < hdr.e_num_phdr; i++) {
        // reads all the phdrs headers
        if (!read_phdr(f, hdr.e_phdr_start + i * sizeof(elf_phdr_t), &phdrs[i])) {
            printf("Failed to read file\n");
            return EXIT_FAILURE;
        }
    }
    //prints all the phdrs details
    if(showphdrs) {
        dump_phdrs(hdr.e_num_phdr, phdrs);
    }

    //loads all the segments into memory, if it fails it will return false and exit with failure
    if (loadmem) {
        byte_t *memory = calloc(MEMSIZE, sizeof(byte_t));
        for (int i = 0; i < hdr.e_num_phdr; i++) {
            if (!load_segment(f, memory, &phdrs[i])) {
                free(memory);
                printf("Failed to read file\n");
                return EXIT_FAILURE;
            }
        }
        //execute and print state after executing instruction
        if(execute){
            y86_t cpu = {0}; //main cpu struct
            bool cnd = false; // conditional signal
            y86_reg_t valA = 0, valE = 0; //intermediate registers
            int execCount = 0; //execution count

            cpu.stat = AOK; //set status to AOK to start execution
            cpu.pc = hdr.e_entry;
            printf("Beginning execution at 0x%04lx\n", cpu.pc);
            while(cpu.stat == AOK) {
                y86_inst_t ins = fetch(&cpu, memory); //fetch instruction?
                    if (cpu.stat != AOK) {
                        break;
                    }
                execCount++;
                valE = decode_execute(&cpu, &ins, &cnd, &valA);
                memory_wb_pc(&cpu, &ins, memory,  cnd, valA, valE);
            }
            dump_cpu_state(&cpu);
            printf("Total execution count: %d\n", execCount);
        }
        //execute with trace mode and print state after every instruction 
       if(trace) {
            y86_t cpu = {0};
            bool cnd = false;
            y86_reg_t valA = 0, valE = 0;
            int execCount = 0;
            y86_inst_t ins = {0};

            cpu.stat = AOK;
            cpu.pc = hdr.e_entry;
            printf("Beginning execution at 0x%04lx\n", cpu.pc);
            dump_cpu_state(&cpu);
            while(cpu.stat == AOK) {
                ins = fetch(&cpu, memory);
                    if (cpu.stat != AOK) {
                        printf("\nInvalid instruction at 0x%04lx\n", cpu.pc);
                        dump_cpu_state(&cpu);
                        break;
                    }
                printf("\n");
                printf("Executing: ");
                disassemble(&ins);
                printf("\n");
                
                valE = decode_execute(&cpu, &ins, &cnd, &valA);
                memory_wb_pc(&cpu, &ins, memory, cnd, valA, valE);
                execCount++;
                
                dump_cpu_state(&cpu);
                //same as execute but with more print statements to show the instruction being executed and the state after every instruction instead of just at the end
            }
            printf("Total execution count: %d\n", execCount);
            printf("\n");
        }

        if (showmembrief) {
            for (int i = 0; i < hdr.e_num_phdr; i++) {
                dump_memory(memory, phdrs[i].p_vaddr, phdrs[i].p_vaddr + phdrs[i].p_size);
            }
        }
        if (showmemfull) {
            dump_memory(memory, 0, MEMSIZE);
        }
        //prints code if thats what is inputed
        if (disCode) {
            printf("Disassembly of executable contents:\n");
            for (int i = 0; i < hdr.e_num_phdr; i++) {
                if (phdrs[i].p_type == CODE) {
                    disassemble_code(memory, &phdrs[i], &hdr);
                    printf("\n");
                }
            }
        }
        //print data if asked
        if (disDat) {
            printf("Disassembly of data contents:\n");
            for (int i = 0; i < hdr.e_num_phdr; i++) {
                if (phdrs[i].p_type == DATA) {
                    if (phdrs[i].p_flags == 0x06) {
                        disassemble_data(memory, &phdrs[i]);
                    } else if (phdrs[i].p_flags == 0x04) {
                        disassemble_rodata(memory, &phdrs[i]);
                    } else {
                        printf("Failed to read file\n");
                    }
                    printf("\n");
                }
            }
        }
        free(memory);

    }

    return EXIT_SUCCESS;
}


