/**
 * \addtogroup gbas
 * \{
 * \defgroup outfile Output file
 * \addtogroup outfile
 * \{
 */

#include "outfile.h"

#include "../common/errors.h"

static FILE* f; /**< The output file */

/*========================================================================*//**
 * Sets the output file
 *
 * \param o: The output file
 *//*=========================================================================*/
void set_outfile(FILE* o)
{
    f = o;
}

/*========================================================================*//**
 * Writes the GBOBJECT file header to the output file
 *//*=========================================================================*/
void write_obj_header()
{
    write_data("GBOBJECT", 8);
    write_int32(1);                 /* Format version */
}

/*========================================================================*//**
 * Writes the GBOBJECT file block header to the output file
 *
 * \param type:
 * \param num_entries:
 *//*=========================================================================*/
void write_block_header(blocktype_t type, int num_entries)
{
    write_int16(type);
    write_int32(num_entries);
}

/*========================================================================*//**
 * Writes a single byte of data to the output file
 *
 * \param val: The value to write
 *//*=========================================================================*/
void write_int8(char val)
{
    if (fputc(val, f) != val)
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
    if (fwrite(bytes, 1, 4, f) != 4)
        ccerr(F, "error while writing to the output file");
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
    if (fwrite(bytes, 1, 2, f) != 2)
        ccerr(F, "error while writing to the output file");
}

/*========================================================================*//**
 * Writes a block of data to the output file
 *
 * \param data: Pointer to the data to write
 * \param size: Size of the block of data
 *//*=========================================================================*/
void write_data(char* data, size_t size)
{
    if (fwrite(data, 1, size, f) != size)
        ccerr(F, "error while writing to the output file");
}

/**
 * \} outfile
 * \} gbas
 */
