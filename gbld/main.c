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
#include "version.h"
#include "map.h"
#include "rom.h"

#define MAX_ID_LEN  31

typedef enum
{
    sections,
    symbols
} blocktype_t;

typedef struct
{
    int     file_id;
    int     section_id;
    int     offset;
    char    id[MAX_ID_LEN + 1];
    int     global;
    int     extern_;
} symbol_t;

const char* const pgm = "gbld";

static FILE*  infile = NULL;

void help();
void version();
void on_fatal_error(int from_program);
unsigned char read_int8();
int  read_int32();
int  read_int16();
void read_data(char* dest, size_t size);


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


int main(int argc, char** argv)
{
    int i, j;
    sourcefile_t* file = NULL;
    char* output_name = NULL;
    FILE* outfile = NULL;
    int file_id = 0;

    esetprogram(pgm);
    esetonfatal(&on_fatal_error);

    parse_options(argc, argv, GBLD, &help, &version);

    if (errors())
        return EXIT_FAILURE;

    output_name = (char*)mmalloc(strlen(get_option("-o")->value.str) + 1);
    strcpy(output_name, get_option("-o")->value.str);

    init_rom();
    add_section(&header);

    file_first();
    while ((file = file_next()))
    {
        size_t fsize;
        char signature[8];
        int  ver;

        esetfile(file->name);

        if (! (infile = fopen(file->name, "rb")))
        {
            ccerr(F, "unable to open \"%s\"", file->name);
            continue;
        }

        fseek(infile, 0, SEEK_END);
        fsize = ftell(infile);
        fseek(infile, 0, SEEK_SET);

        if (fread(signature, 1, 8, infile) < 8)
            err(F, "file format not recognized");

        if (strncmp(signature, "GBOBJECT", 8))
            err(F, "file format not recognized");

        ver = read_int32();
        if (ver != 1)
            ccerr(F, "object file format version not handled");

        while (ftell(infile) < fsize)
        {
            blocktype_t type = read_int16();
            int num_entries = read_int32();


            if (type == sections)
            {
                    err(N, "New sections block with %d entries", num_entries);
                for (i = 0; i < num_entries; ++i)
                {
                    /* check section size ###########################################" */

                    section_t* sect = mmalloc(sizeof(section_t));
                    sect->id = read_int32();
                    sect->type = read_int16();
                    sect->address = read_int16();
                    sect->datasize = read_int32();
                    sect->data = (char*)mmalloc(sect->datasize);
                    read_data(sect->data, sect->datasize);

                    sect->reloc_address = sect->address;
                    sect->file_id = file_id;
                    sect->header = 0;

                    add_section(sect);
                }
            }
            else if (type == symbols)
            {
                err(N, "New symbols block with %d entries", num_entries);
                for (i = 0; i < num_entries; ++i)
                {
                    symbol_t sym;
                    read_data((char*)&sym.id, MAX_ID_LEN + 1);
                    sym.section_id = read_int32();
                    sym.offset = read_int16();
                    sym.global = read_int8();
                    sym.extern_ = read_int8();

                    sym.file_id = file_id;

                    if (sym.global)
                        fprintf(stderr, "global\n");
                    if (sym.extern_)
                        fprintf(stderr, "extern\n");
                    fprintf(stderr, "name: %s\n", sym.id);
                    fprintf(stderr, "section: %d\n", sym.section_id);
                    fprintf(stderr, "offset: %d\n", sym.offset);

                    fprintf(stderr, "\n");
                }
            }
            else
                err(F, "invalid object file");
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
    free_sections();

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
    free_sections();
    exit(EXIT_FAILURE);
}




unsigned char read_int8()
{
    unsigned char val;
    if (fread(&val, 1, 1, infile) < 1)
        err(F, "invalid object file");
    return val;
}



int read_int32()
{
    unsigned char bytes[4];
    if (fread(bytes, 1, 4, infile) < 4)
        err(F, "invalid object file");
    return ((bytes[3] << 24) + (bytes[2] << 16) + (bytes[1] << 8) + bytes[0]);
}




int read_int16()
{
    unsigned char bytes[2];
    if (fread(bytes, 1, 2, infile) < 2)
        err(F, "invalid object file");
    return ((bytes[1] << 8) + bytes[0]);
}




void read_data(char* dest, size_t size)
{
    if (fread(dest, 1, size, infile) < size)
        err(F, "invalid oject file");
}

/**
 * \}
 */
