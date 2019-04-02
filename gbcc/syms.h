#ifndef SYMS_H
#define SYMS_H

#include "lexer.h"

/** Symbol entry in a symbol table */
typedef struct sym_s
{
    char id[MAX_ID_LEN + 1];    /**< Identifier name */
    int  type_id;               /**< BYTE, WORD, TYPENAME... return type in case of a function */
    int  function;
} sym_t;


typedef struct symtbl_s symtbl_t;

/** Symbol table */
struct symtbl_s
{
    int          scope_id;      /**< ID of the table's scope */
    unsigned int num_syms;      /**< Number of symbols in this table */
    unsigned int num_children;  /**< Number of child tables */
    sym_t**      syms;          /**< Symbols list */
    symtbl_t**   children;      /**< Child tables list */
    symtbl_t*    parent;        /**< Parent table */
};



void      syms_init();
void      syms_free();
void      syms_set_cur_scope(int scope_id);
symtbl_t* syms_get_table();
sym_t*    create_sym();
sym_t*    get_sym(const char* id);
void      syms_add(sym_t* s);
int       syms_inc_scope();
void      syms_dec_scope();


#include <stdio.h>
void syms_dump(FILE* f);

#endif
