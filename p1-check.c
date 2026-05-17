/*
 * CS 261 PA1: Mini-ELF header verifier
 *
 * Name: Aayush Patel
 */

#include "p1-check.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

bool read_header (FILE *file, elf_hdr_t *hdr)
{
    if (file == NULL || hdr == NULL) {
        //null check
        printf("Failed to read file\n");
        return false;
    }
    int nread = fread(hdr, sizeof(*hdr), 1, file);
    //reads 1, 16 btye (cause thats how elf_hdr_t was set) chunk from the file where hdr is pointing

    if (nread != 1) {
        printf("Failed to read file\n");
        return false;
        //returns false if it did not read the file correctly
    }
    if (hdr->magic != 0x00464c45) {
        printf("Failed to read file\n");
        return false;
        //return false if magic nymber is wrong
    }

    return true;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void dump_header (elf_hdr_t *hdr)
{
    unsigned char *h = (unsigned char *)hdr;
    //store binary
    //unsigned because thats how it is in the elf doc
    for (int i = 0; i<= (sizeof(*hdr) - 1); i++) {
        printf("%02x", h[i]);
        //loops through h to get all 16 bytes of the header binary
        if (i != (sizeof(*hdr) - 1)) {
            printf(" ");
            //hardcoded spacing to match the tests will review to try and find a better way but idk rn
        }
        if (i == 7) {
            printf(" ");
            // double hardcoded the spacing same thing tho^
        }
    }
    //formatted prints for the fields we just read
    printf("\nMini-ELF version %u\n", hdr->e_version);
    printf("Entry point 0x%02x\n", hdr->e_entry);
    printf("There are %u program headers, starting at offset %u (0x%02x)\n",hdr->e_num_phdr,
           hdr->e_phdr_start, hdr->e_phdr_start);
    if (hdr->e_symtab == 0) {
        printf("There is no symbol table present\n");   //checks if symbol table is present
    } else {
        printf("There is a symbol table starting at offset %u (0x%02x)\n", hdr->e_symtab,  hdr->e_symtab);
    }
    if (hdr->e_strtab == 0) {
        printf("There is no string table present\n");   //checks if string table is present
    } else {
        printf("There is a string table starting at offset %u (0x%02x)\n", hdr->e_strtab,  hdr->e_strtab);
    }
}

