/**
 * \addtogroup gbas
 * \{
 * \addtogroup outfile
 * \{
 */

#ifndef OUTFILE_H
#define OUTFILE_H

#include <stdio.h>

typedef enum
{
    sections,
    symbols
} blocktype_t;

void set_outfile(FILE* o);
void write_obj_header();
void write_block_header(blocktype_t type, int num_entries);
void write_int8(char val);
void write_int16(int val);
void write_int32(int val);
void write_data(char* data, size_t size);

#endif

/**
 * \outfile
 * \gbas
 */
