/*
 * CS 261 PA2: Mini-ELF loader
 *
 * Name: Aayush Patel
 */

#include "p2-load.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

bool read_phdr (FILE *file, uint16_t offset, elf_phdr_t *phdr)
{
    if (file == NULL || phdr == NULL) {
        //null check
        printf("Failed to read file\n");
        return false;
    }
    if (fseek(file, offset, SEEK_SET) != 0) {
        //jumps to the offset, error if unsuccessful
        printf("Failed to read file\n");
        return false;
    }

    //reads 1, elf_phdr sized chunk from the file where phdr is pointing
    if (fread(phdr, sizeof(elf_phdr_t), 1, file)  != 1) {
        printf("Failed to read file\n");
        return false;
        //returns false if it did not read the file correctly
    }
    if (phdr->magic != 0xDEADBEEF) {
        //checks if the magic number is correct
        printf("Failed to read file\n");
        return false;
    }

    return true;
}

bool load_segment (FILE *file, byte_t *memory, elf_phdr_t *phdr)
{
    if (file == NULL || memory == NULL || phdr == NULL) {
        //null check
        printf("Failed to read file\n");
        return false;
    }
    // If segment starts out of bounds, fail
    if (phdr->p_vaddr >= MEMSIZE) {
        printf("Failed to read file\n");
        return false;
    }

    // If vaddr + size would overflow or exceed memory, fail
    if (phdr->p_size > MEMSIZE - phdr->p_vaddr) {
        printf("Failed to load segment\n");
        return false;
    }

    if (phdr->p_size == 0) {
        return true;  // nothing to load, that's fine
    }

    //moves the memory pointer to the virtual address
    if (fseek(file, phdr->p_offset, SEEK_SET) != 0) {
        printf("Failed to load segment\n");
        return false;
    }
    //read one p_sized chunk from file to memory
    if (fread(&memory[phdr->p_vaddr], phdr->p_size, 1, file) != 1) {
        printf("Failed to load segment\n");
        return false;
    }
    return true;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void dump_phdrs (uint16_t numphdrs, elf_phdr_t *phdrs)
{
    if (phdrs == NULL) {
        return;
    }
    //null check
    printf(" Segment   Offset    Size      VirtAddr  Type      Flags\n");
    char *type;
    char *flag;
    //code for getting the right types and the right read write directions from the flags
    for (uint16_t i = 0; i < numphdrs; i++) {
        if (phdrs[i].p_type == 1) {
            type = "CODE";
        } else if (phdrs[i].p_type == 0) {
            type = "DATA";
        } else if (phdrs[i].p_type == 2) {
            type = "STACK";
        } else if (phdrs[i].p_type == 3) {
            type = "HEAP";
        } else {
            type = "UNKNOWN";
        }
        if (phdrs[i].p_flags == 0) {
            flag = "   ";
        } else if (phdrs[i].p_flags == 1) {
            flag = "  X";
        } else if (phdrs[i].p_flags == 2) {
            flag = " W ";
        } else if (phdrs[i].p_flags == 3) {
            flag = " WX";
        } else if (phdrs[i].p_flags == 4) {
            flag = "R  ";
        } else if (phdrs[i].p_flags == 5) {
            flag = "R X";
        } else if (phdrs[i].p_flags == 6) {
            flag = "RW ";
        } else if (phdrs[i].p_flags == 7) {
            flag = "RWX";
        } else {
            flag = "???";
        }
        printf("  %02u       0x%04x    0x%04x    0x%04x    %-8s  %3s\n",
               (unsigned)i,
               (unsigned)phdrs[i].p_offset,
               (unsigned)phdrs[i].p_size,
               (unsigned)phdrs[i].p_vaddr,
               type,
               flag);
    }
}
void dump_memory (byte_t *memory, uint16_t start, uint16_t end)
{
    printf("Contents of memory from %04x to %04x:\n", start, end);
    if (start == end) {
        return; // if start and end are the same, there is nothing to print
    }
    uint16_t row_start = start - (start % 16); // align start to the nearest 16-byte boundary
    printf("  %04x  ", row_start);
    for (uint16_t i = row_start; i < end; i++) {
        if (i < start) {
            // pad with spaces before actual data to fix unaligned ones
            printf("  ");  // empty byte position
            if ((i - row_start + 1) % 8 == 0) {
                printf("  ");
            } else if (i + 1 < end) {
                printf(" ");
            }
        } else {
            printf("%02x", memory[i]);
            // print the byte
            if ((i - row_start + 1) % 16 == 0) {
                if (i + 1 < end) {
                    printf("\n  %04x  ", i + 1);
                }
                //print the memory address every new line
            } else if ((i - row_start + 1) % 8 == 0) {
                printf("  ");
                // extra space cause thats how the tests are
            } else if (i + 1 < end) {
                printf(" ");
                // space between bytes except not after the last one
            }
        }
    }
    printf("\n");


}