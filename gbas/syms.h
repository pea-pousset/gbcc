/**
 * \addtogroup gbas
 * \{
 * \addtogroup Symbols
 * \{
 */

#ifndef SYMS_H
#define SYMS_H

#include "../common/objfile.h"

#define MAX_ID_LEN 31


typedef struct sym_s
{
    int        sym_id;
    char       id[MAX_ID_LEN + 1]; /**< Symbol's identifier */
    int        section_id;         /**< ID of the section containing the sym */
    int        offset;             /**< Address of the symbol in the section */
    sym_type_t type;               /**< Symbol's type (none, global, extern) */
    char*      filename;
    int        line;
    int        column;
    struct sym_s* next;
} sym_t;

void   init_syms();
void   free_syms();
sym_t* sym_get_table();
void   sym_declare(int pass, char* id, char* filename, int line, int column);
void   sym_set_global(int pass, char* id);
int    sym_request(int pass, char* id, int relative);
void   write_syms();

#endif

/**
 * \} Symbols
 * \} gbas
 */
