/**
 * \addtogroup Commons
 * \{
 * \defgroup objfile Object file
 * \addtogroup objfile
 * \}
 */

#include "objfile.h"

#include <strings.h>

#include "errors.h"
#include "utils.h"

static FILE* in = NULL;
static FILE* out = NULL;

static unsigned char read_int8();
static int           read_int16();
static int           read_int32();
static void          read_data(unsigned char* dest, size_t size);

static void write_int16(int val);
static void write_int32(int val);
static void write_data(unsigned char* data, size_t size);

void set_infile(FILE* infile)
{
    in = infile;
}

void set_outfile(FILE* outfile)
{
    out = outfile;
}

void read_obj_header()
{
    obj_header_t header;
    read_data((unsigned char*)header.signature, 8);
    header.version = read_int32();
    if (strncmp((char*)header.signature, "GBOBJECT", 8) != 0)
        err(F, "invalid object file");
    if (header.version != 1)
        err(F, "object file format version not handled");
}

block_header_t* read_block_header()
{
    block_header_t* header = (block_header_t*)mmalloc(sizeof(block_header_t));
    header->type = read_int32();
    header->num_entries = read_int32();
    if (header->type < sections || header->type > relocations)
        err(F, "invalid object file: unknown block type");
    return header;
}

section_entry_t* read_section_entry()
{
    section_entry_t* sect = (section_entry_t*)mmalloc(sizeof(section_entry_t));
    sect->id = read_int32();
    sect->type = read_int32();
    sect->address_or_bank = read_int16();
    sect->data_size = read_int32();
    if (sect->type < org || sect->type > org)
        err(F, "invalid object file: unknown section type");
    sect->data = (unsigned char*)mmalloc(sect->data_size);
    read_data(sect->data, sect->data_size);
    return sect;
}

symbol_entry_t* read_symbol_entry()
{
    symbol_entry_t* sym = (symbol_entry_t*)mmalloc(sizeof(symbol_entry_t));
    sym->sym_id = read_int32();
    read_data((unsigned char*)sym->id, 32);
    sym->section_id = read_int32();
    sym->offset = read_int16();
    sym->type = read_int32();
    if (sym->type < none || sym->type > _extern)
        err(F, "invalid object file: unknown symbol type");
    return sym;
}

reloc_entry_t* read_reloc_entry()
{
    reloc_entry_t* reloc = (reloc_entry_t*)mmalloc(sizeof(reloc_entry_t));
    reloc->sym_id = read_int32();
    reloc->section_id = read_int32();
    reloc->offset = read_int16();
    reloc->flags = read_int32();
    return reloc;
}

void write_obj_header()
{
    obj_header_t header;
    strncpy((char*)header.signature, "GBOBJECT", 8);
    header.version = 1;
    write_data((unsigned char*)header.signature, 8);
    write_int32(header.version);
}

void write_block_header(block_header_t* header)
{
    write_int32(header->type);
    write_int32(header->num_entries);
}

/*========================================================================*//**
 * Write a section entry header to the output file
 *
 * \param entry: section entry header
 *
 * \warning data are NOT written
 *//*=========================================================================*/
void write_section_entry(section_entry_t* entry)
{
    write_int32(entry->id);
    write_int32(entry->type);
    write_int16(entry->address_or_bank);
    write_int32(entry->data_size);
}

void write_symbol_entry(symbol_entry_t* entry)
{
    write_int32(entry->sym_id);
    write_data(entry->id, 32);
    write_int32(entry->section_id);
    write_int16(entry->offset);
    write_int32(entry->type);
}

void write_reloc_entry(reloc_entry_t* entry)
{
    write_int32(entry->sym_id);
    write_int32(entry->section_id);
    write_int16(entry->offset);
    write_int32(entry->flags);
}

/*========================================================================*//**
 * Writes a single byte of data to the output file
 *
 * \param val: The value to write
 *//*=========================================================================*/
void write_byte(unsigned char val)
{
    if (fputc(val, out) != val)
        ccerr(F, "error while writing to the output file");
}

unsigned char read_int8()
{
    unsigned char val;
    if (fread(&val, 1, 1, in) < 1)
        err(F, "invalid object file");
    return val;
}

int read_int16()
{
    unsigned char bytes[2];
    if (fread(bytes, 1, 2, in) < 2)
        err(F, "invalid object file");
    return ((bytes[1] << 8) + bytes[0]);
}

int read_int32()
{
    unsigned char bytes[4];
    if (fread(bytes, 1, 4, in) < 4)
        err(F, "invalid object file");
    return ((bytes[3] << 24) + (bytes[2] << 16) + (bytes[1] << 8) + bytes[0]);
}

void read_data(unsigned char* dest, size_t size)
{
    if (fread(dest, 1, size, in) < size)
        err(F, "invalid oject file");
}

/*========================================================================*//**
 * Writes a 16 bits big-endian value to the output file
 *
 * \param val: The value to write
 *//*=========================================================================*/
void write_int16(int val)
{
    char bytes[2];
    bytes[0] = ((val & 0x00FF));
    bytes[1] = ((val & 0xFF00) >> 8);
    if (fwrite(bytes, 1, 2, out) != 2)
        ccerr(F, "error while writing to the output file");
}

/*========================================================================*//**
 * Writes a 32 bits big-endian value to the output file
 *
 * \param val: The value to write
 *//*=========================================================================*/
void write_int32(int val)
{
    char bytes[4];
    bytes[0] = ((val & 0x000000FF));
    bytes[1] = ((val & 0x0000FF00) >> 8);
    bytes[2] = ((val & 0x00FF0000) >> 16);
    bytes[3] = ((val & 0xFF000000) >> 24);
    if (fwrite(bytes, 1, 4, out) != 4)
        ccerr(F, "error while writing to the output file");
}

/*========================================================================*//**
 * Writes a block of data to the output file
 *
 * \param data: Pointer to the data to write
 * \param size: Size of the block of data
 *//*=========================================================================*/
void write_data(unsigned char* data, size_t size)
{
    if (fwrite(data, 1, size, out) != size)
        ccerr(F, "error while writing to the output file");
}

/**
 * \} objfile
 * \} Commons
 */
