/**
 * \defgroup gbld gbld
 * Linker
 * \addtogroup gbld
 * \{
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/errors.h"
#include "../common/files.h"
#include "../common/options.h"
#include "../common/utils.h"
#include "../common/objfile.h"
#include "version.h"
#include "map.h"
#include "rom.h"

typedef struct sectlist_s
{
    int                file_id;
    section_entry_t*   entry;
    struct sectlist_s* next;
} sectlist_t;

const char* const pgm = "gbld";

static FILE*  infile = NULL;

void help();
void version();
void on_fatal_error(int from_program);
void add_section_entry(int file_id, section_entry_t* entry);

const char header_data[76] =
{
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
    'O', 'U', 'T', '.', 'G', 'B', ' ', ' ', ' ', ' ', ' ',  /* Title */
    0xBA, 0xAD, 0xF0, 0x0D,     /* Manufacturer code */
    0,                          /* GameBoy Color compatibilty */
    ' ', ' ',                   /* New licensee code */
    0,                          /* Super Gameboy compatibility */
    0,                          /* Cartridge Type */
    0,                          /* Rom size */
    0,                          /* Ram size */
    1,                          /* Destination code */
    0x33,                       /* Old licensee code */
    0,                          /* Rom version */
    0xFF,                       /* Header checksum */
    0xFF, 0xFF                  /* Global checksum */
};
/*
const section_t header =
{
    .file_id       = -1,
    .id            = 0,
    .type          = org,
    .address       = 0x104,
    .reloc_address = 0x104,
    .header        = 1,
    .datasize      = 76,
    .data          = header_data
};

*/

int main(int argc, char** argv)
{
    int i, j, file_id = 0;
    sourcefile_t* file = NULL;
    char* output_name = NULL;
    FILE* outfile = NULL;

    esetprogram(pgm);
    esetonfatal(&on_fatal_error);

    parse_options(argc, argv, GBLD, &help, &version);

    if (errors())
        return EXIT_FAILURE;

    output_name = (char*)mmalloc(strlen(get_option("-o")->value.str) + 1);
    strcpy(output_name, get_option("-o")->value.str);

    init_rom();

    file_first();
    while ((file = file_next()))
    {
        size_t fsize;
        esetfile(file->name);
        if (! (infile = fopen(file->name, "rb")))
        {
            ccerr(F, "unable to open \"%s\"", file->name);
            continue;
        }

        fseek(infile, 0, SEEK_END);
        fsize = ftell(infile);
        fseek(infile, 0, SEEK_SET);
        
        set_infile(infile);
        read_obj_header();
        
        while (ftell(infile) < fsize)
        {
            block_header_t* header= read_block_header();
            if (header->type == sections)
            {
                section_entry_t* sect;
#ifndef NDEBUG
                printf("New sections block with %d entries\n", header->num_entries);
#endif
                for (i = 0; i < header->num_entries; ++i)
                {
                    sect = read_section_entry();
#ifndef NDEBUG
                    printf("  * Section %d\n", sect->id);
                    printf("    - Type:      %s\n", sect->type == org ? ".org" : "");
                    printf("    - Address:   %04x\n", sect->address_or_bank);
                    printf("    - Data size: %d\n", sect->data_size);
                    printf("\n");
#endif
                }
            }
            else if (header->type == symbols)
            {
                symbol_entry_t* sym;
#ifndef NDEBUG
                printf("New symbols block with %d entries\n", header->num_entries);
#endif
                for (i = 0; i < header->num_entries; ++i)
                {
                    sym = read_symbol_entry();
#ifndef NDEBUG
                    printf("  * Symbol %d\n", sym->sym_id);
                    printf("    - ID:         %s\n", sym->id);
                    printf("    - In section: %d\n", sym->section_id);
                    printf("    - Offset:     %d\n", sym->offset);
                    printf("    - Type:       %c\n", " GE"[sym->type]);
                    printf("\n");
#endif
                }
            }
            else if (header->type == relocations)
            {
                reloc_entry_t* reloc;
#ifndef NDEBUG
                printf("New relocations block with %d entries\n", header->num_entries);
#endif
                for (i = 0; i < header->num_entries; ++i)
                {
                    reloc = read_reloc_entry();
#ifndef NDEBUG
                    printf("  * Relocation\n");
                    printf("  - Symbol: %d\n", reloc->sym_id);
#endif
                }
            }
        }

        fclose(infile);
        ++file_id;
    }

    if ( ! (outfile = fopen(output_name, "wb")) )
        ccerr(F, "unable to open \"%s\"", output_name);

        err(N, "%s", output_name);

     for (i = 0; i < 2; ++i) /* NUM BANKS */
        fwrite(get_rom_bank(i), BANK_SIZE, 1, outfile);

    fclose(outfile);
    free_rom();
    // free_sections();

    return EXIT_SUCCESS;
}




void help()
{
    puts("Usage: gbld [options] file...");
    puts("Options:");
    puts("  --help      Display this information");
    puts("  --version   Display linker version information");
    puts("  -o <file>   Place the output into <file>");
    exit(EXIT_SUCCESS);
}




void version()
{
    printf("%s %d.%d\n", pgm, VERSION_MAJOR, VERSION_MINOR);
    exit(EXIT_SUCCESS);
}




void on_fatal_error(int from_program)
{
    free_rom();
    exit(EXIT_FAILURE);
}

/**
 * \}
 */
