/**
 * \addtogroup Commons
 * \{
 * \addtogroup objfile
 * \{
 */

#ifndef OBJFILE_H
#define OBJFILE_H

#include <stdio.h>

typedef enum
{
    sections,
    symbols,
    relocations
} block_type_t;

typedef enum
{
    org
} section_type_t;

typedef enum
{
    none,
    _global,
    _extern
} sym_type_t;

enum reloc_flags
{
    relative = 0x01
};

typedef struct obj_header_s
{
    unsigned char signature[8];
    int           version;
} obj_header_t;

typedef struct block_header_s
{
    block_type_t type;
    int          num_entries;
} block_header_t;

typedef struct section_entry_s
{
    int            id;
    section_type_t type;
    int            offset;
    int            bank_num;
    int            data_size;
    unsigned char* data;
} section_entry_t;

typedef struct symbol_entry_s
{
    int           sym_id;
    unsigned char id[32];
    int           section_id;
    int           offset;
    sym_type_t    type;
} symbol_entry_t;

typedef struct reloc_entry_s
{
    int sym_id;     /**< id of the pointed symbol */
    int section_id; /**< id of the section containing the address to relocate */
    int offset;     /**< offset in the section of the address to relocate */
    int flags;      /**< relocation attributes flag */
} reloc_entry_t;

void             set_infile(FILE* infile);
void             set_outfile(FILE* outfile);
void             read_obj_header();
block_header_t*  read_block_header();
section_entry_t* read_section_entry();
symbol_entry_t*  read_symbol_entry();
reloc_entry_t*   read_reloc_entry();
void             write_obj_header();
void             write_block_header(block_header_t* header);
void             write_section_entry(section_entry_t* entry);
void             write_symbol_entry(symbol_entry_t* entry);
void             write_reloc_entry(reloc_entry_t* reloc);
void             write_byte(unsigned char val);

#endif

/**
 * \} objfile
 * \} Commons
 */
