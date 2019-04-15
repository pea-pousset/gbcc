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
#include "../common/gbmmap.h"
#include "version.h"
#include "map.h"
#include "rom.h"
#include "lists.h"

const char* const pgm = "gbld";

static FILE*  infile = NULL;

void help();
void             version();
void             on_fatal_error(int from_program);
section_entry_t* get_section(int file_id, unsigned sect_id);
void             write_section(section_entry_t* sect);

int main(int argc, char** argv)
{
    int i, file_id = 0;
    unsigned numsect = 0, numsyms = 0, numrelocs = 0;
    list_t* list;
    sourcefile_t* file = NULL;
    char* output_name = NULL;
    FILE* outfile = NULL;
    list_t* psyms1, * psyms2;


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
        char* filename;
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
        
        filename = (char*)mmalloc(strlen(file->name) + 1);
        strcpy(filename, file->name);
        list_add(&lfiles, file_id, filename, 0);
        
        while (ftell(infile) < fsize)
        {
            block_header_t* header= read_block_header();
            init_map();
            
            if (header->type == sections)
            {
                section_entry_t* sect;
#ifndef NDEBUG
                printf("New sections block with %d entries\n", header->num_entries);
#endif
                for (i = 0; i < header->num_entries; ++i)
                {
                    sect = read_section_entry();
                    list_add(&lsections, file_id, sect, 0);
                    ++numsect;
#ifndef NDEBUG
                    printf("  * Section %d\n", sect->id);
                    printf("    - Type:      %s\n", sect->type == org ? ".org" : "");
                    printf("    - Offset:    %04x\n", sect->offset);
                    printf("    - Bank:      %d\n", sect->bank_num);
                    printf("    - Data size: %d\n", sect->data_size);
                    for (int k = 0; k < sect->data_size; ++k)
                    {
                        if (k && (k % 16) == 0)
                            printf("\n");
                        else if (k && (k % 8) == 0)
                            printf(" ");
                        printf("%02X ", sect->data[k]);

                    }
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
                    list_add(&lsymbols, file_id, sym, file_id);
                    ++numsyms;
#ifndef NDEBUG
                    printf("  * Symbol %d\n", sym->sym_id);
                    printf("    - ID:         %s\n", sym->id);
                    printf("    - In section: %d\n", sym->section_id);
                    printf("    - Offset:     %d\n", sym->offset);
                    printf("    - Type:       %c\n", ".GE"[sym->type]);
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
                    ++numrelocs;
                    list_add(&lrelocations, file_id, reloc, 0);
#ifndef NDEBUG
                    printf("  * Relocation\n");
                    printf("    - Symbol: %d\n", reloc->sym_id);
                    printf("    - Section: %d\n", reloc->section_id);
                    printf("    - Offset: %d\n", reloc->offset);
#endif
                }
            }
        }

        fclose(infile);
        ++file_id;
    }
    
    /* Search for duplicate global symbols */
    psyms1 = lsymbols;
    while (psyms1)
    {
        psyms2 = psyms1->next;
        while (psyms2)
        {
            if (psyms1->file_id == psyms2->file_id)
            {
                psyms2 = psyms2->next;
                continue;
            }
            if (((symbol_entry_t*)(psyms1->data))->type != _global
                || ((symbol_entry_t*)(psyms2->data))->type != _global)
            {
                psyms2 = psyms2->next;
                continue;
            }
            if (strcmp((const char*)((symbol_entry_t*)(psyms1->data))->id,
                       (const char*)((symbol_entry_t*)(psyms2->data))->id)
                == 0)
            {
                ccerr(E, "duplicate symbol '%s'",
                    ((symbol_entry_t*)(psyms2->data))->id);
            }
            
            psyms2 = psyms2->next;
        }
        psyms1 = psyms1->next;
    }
    
    /* .org allocation */
    list = lsections;
    while (list)
    {
        list_t* sfile = list_at(lfiles, list->file_id);
        section_entry_t* sect = (section_entry_t*)list->data;
        esetfile(sfile->data);
        
        if (sect->type != org)
        {
            list = list->next;
            continue;
        }
        
        sect->offset = allocate(sfile->data, sect);
        list->flags = SECT_TREATED;
        
        list = list->next;
    }
    
    TODO("rom/ram/wram/vram + offset");
    TODO("rom/ram/wram/vram");
    
    /* link extern symbols */
    list = lsymbols;
    while(list)
    {
        list_t* sfile = list_at(lfiles, list->file_id);
        symbol_entry_t* sym = (symbol_entry_t*)(list->data);
        list_t* tsyms = lsymbols; /* target symbols list */
        symbol_entry_t* tsym;     /* target symbol */
        
        if (sym->type != _extern)
        {
            list = list->next;
            continue;
        }
        
        esetfile((char*)sfile->data);
        while (tsyms)
        {
            tsym = (symbol_entry_t*)(tsyms->data);
            if (list->file_id == tsyms->file_id || tsym->type != _global)
            {
                tsyms = tsyms->next;
                continue;
            }
            
            if (strcmp((char*)sym->id, (char*)tsym->id) == 0)
                break;
            
            tsyms = tsyms->next;
        }
        
        if (tsyms == NULL)
            err(E, "symbol '%s' unsolved", sym->id);
        else
        {
            list->flags = tsyms->file_id; /* flag = target file id */
            sym->section_id = tsym->section_id;
            sym->offset = tsym->offset;
        }
        
        list = list->next;
    }
    
    /* relocs */
    list = lrelocations;
    while (list)
    {
        list_t* lfile = list_at(lfiles, list->file_id);
        reloc_entry_t* reloc = (reloc_entry_t*)(list->data);
        list_t* target_syms = lsymbols;
        symbol_entry_t* target_sym;
        section_entry_t* reloc_sect;
        section_entry_t* symbol_sect;
        int target_addr;
        
        esetfile((char*)lfile->data);

        /* search for target symbol among symbols created in the same file */
        while (target_syms)
        {
            if (target_syms->file_id != list->file_id)
            {
                target_syms = target_syms->next;
                continue;
            }
            if (((symbol_entry_t*)(target_syms->data))->sym_id == reloc->sym_id)
                break;
            target_syms = target_syms->next;
        }
        
        if (target_syms == NULL)
            err(F, "relocation entries corrupted");
        
        target_sym = (symbol_entry_t*)(target_syms->data);
        reloc_sect = get_section(list->file_id, reloc->section_id);
        if (target_sym->type == _extern)
            symbol_sect = get_section(target_syms->flags, target_sym->section_id);
        else
            symbol_sect = get_section(target_syms->file_id, target_sym->section_id);
        
        
        target_addr = symbol_sect->offset + target_sym->offset;
        if (reloc->flags == relative)
        {
            int jr = target_addr - (reloc_sect->offset + reloc->offset + 1);
            reloc_sect->data[reloc->offset] = jr;
            if (jr > 128 || jr < -127)
                err(E, "relative jump to '%s' out of reach", target_sym->id);
        }
        else
        {
            reloc_sect->data[reloc->offset] = target_addr & 0xFF;
            reloc_sect->data[reloc->offset + 1] = (target_addr >> 8) & 0xFF;
        }
        
        list = list->next;
    }
    
    /* Write sections to rom banks */
    list = lsections;
    while (list && !errors())
    {
        write_section((section_entry_t*)(list->data));
        list = list->next;
    }

    if (!errors())
    {
        if ( ! (outfile = fopen(output_name, "wb")) )
            ccerr(F, "unable to open \"%s\"", output_name);
        
        fix_rom();
        for (i = 0; i < get_num_rom_banks(); ++i)
            fwrite(get_rom_bank(i), ROM_BANK_SIZE, 1, outfile);
        fclose(outfile);
    }
    
    /* Write sym file */
    if (!errors())
    {
        symbol_entry_t* syms;
        section_entry_t* sect;
        int bank_num;
        char* p = output_name;

        while (*p)
            ++p;
        while (*p != '.' && p != output_name)
            --p;
        if (*p == '.')
            *p = 0;
        output_name = (char*)mrealloc(output_name, strlen(output_name) + 4);
        p = output_name;
        while (*p != 0)
            ++p;
        *p++ = '.';
        *p++ = 's';
        *p++ = 'y';
        *p++ = 'm';
        list = lsymbols;
        
        if ( ! (outfile = fopen(output_name, "w")) )
            ccerr(F, "unable to open \"%s\"", output_name);
        
        while (list && !errors())
        {
            syms = (symbol_entry_t*)(list->data);
            if (syms->type != _extern)
            {
                sect = get_section(list->file_id, syms->section_id);
                if (get_space(sect->offset) == rom_n)
                    bank_num = sect->bank_num + 1;
                else
                    bank_num = sect->bank_num;
                fprintf(outfile, "%02X:%04X %s\n", bank_num,
                       sect->offset + syms->offset, syms->id);
            }
            list = list->next;
        }
        
        fclose(outfile);
    }

    free_rom();
    free_map();
    list_free(lfiles);
    list_free(lsections);
    list_free(lsymbols);
    list_free(lrelocations);

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
    free_map();
    list_free(lfiles);
    list_free(lsections);
    list_free(lsymbols);
    list_free(lrelocations);
    exit(EXIT_FAILURE);
}

section_entry_t* get_section(int file_id, unsigned sect_id)
{
    list_t* lsect = lsections;
    section_entry_t* sect;
    while (lsect)
    {
        if (lsect->file_id == file_id)
        {
            sect = (section_entry_t*)(lsect->data);
            if (sect->id == sect_id)
                break;
        }
        lsect = lsect->next;
    }
    if (lsect != NULL)
        return sect;
    return NULL;
}

void write_section(section_entry_t* sect)
{
    unsigned char* src = sect->data, * dest;
    unsigned count = sect->data_size;
    if (sect->offset == ALLOC_FAILED)
        return;
    if (sect->type == org)
    {
        unsigned offset = sect->offset % ROM_BANK_SIZE;
        dest = get_rom_from_org(sect->offset) + offset;
    }
    else
    {
        TODO("write non org sections");
        return;
    }
    
    while(count--)
        *dest++ = *src++;
    
    free(sect->data);
    sect->data = NULL;
}

/**
 * \}
 */
