#ifndef SYMS_H
#define SYMS_H

#define MAX_ID_LEN 31

typedef struct sym_s
{
    char  id[MAX_ID_LEN + 1]; /**< Symbol's identifier */
    int   section_id;         /**< ID of the section containing the symbol */
    int   offset;             /**< Address of the symbol in the section */
    char  global;             /**< Non-zero means the symbol is global */
    char  extern_;            /**< Non-zero means the symbol is imported */
    char* filename;
    int   line;
    int   column;
    struct sym_s* next;
} sym_t;

void init_syms();
sym_t* sym_get_table();
void sym_declare(int pass, char* id, char* filename, int line, int column); /*
void sym_set_global(int pass, char* id); */
int  sym_request(int pass, char* id, int relative);   /*
void sym_request_rel(int pass, char* id); */
void write_syms();

#endif
