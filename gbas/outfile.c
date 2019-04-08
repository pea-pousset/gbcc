/**
 * \addtogroup gbas
 * \{
 * \defgroup outfile Output file
 * \addtogroup outfile
 * \{
 */

#include "outfile.h"

static FILE* f;

void set_outfile(FILE* o)
{
    f = o;
}

void write_obj_header()
{
    fwrite("GBOBJECT", 1, 8, f);    /* Signature */
    write_int32(1);                 /* Format version */
}

void write_block_header(blocktype_t type, int num_entries)
{
    write_int16(type);
    write_int32(num_entries);
}


void write_int8(char val)
{
    fputc(val, f);
}

/*========================================================================*//**
 * Writes a 32 bits big-endian value to the output file
 *
 * \param val: The value to write
 * \todo error handling
 *//*=========================================================================*/
void write_int32(int val)
{
    char bytes[4];
    bytes[0] = ((val & 0x000000FF));
    bytes[1] = ((val & 0x0000FF00) >> 8);
    bytes[2] = ((val & 0x00FF0000) >> 16);
    bytes[3] = ((val & 0xFF000000) >> 24);
    fwrite(bytes, 1, 4, f);
}

/*========================================================================*//**
 * Writes a 16 bits big-endian value to the output file
 *
 * \param val: The value to write
 * \todo error handling
 *//*=========================================================================*/
void write_int16(int val)
{
    char bytes[2];
    bytes[0] = ((val & 0x00FF));
    bytes[1] = ((val & 0xFF00) >> 8);
    fwrite(bytes, 1, 2, f);
}

void write_data(char* data, size_t size)
{
    fwrite(data, 1, size, f);
}

/**
 * \} outfile
 * \} gbas
 */
