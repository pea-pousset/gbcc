/**
 * \addtogroup gbas
 * \{
 * \defgroup Symbols
 * \addtogroup Symbols
 * \{
 */

#include "syms.h"

#include <stdlib.h>

#include "../common/errors.h"
#include "../common/utils.h"
#include "../common/objfile.h"
#include "commons.h"
#include "sections.h"
#include "relocs.h"

static sym_t* root = NULL;  /**< Root of the symbols list */
static sym_t* cur = NULL;   /**< Current symbol */
static int num_syms;        /**< Number of symbols in the list */

/*========================================================================*//**
 * Initialize the symbol table
 *//*=========================================================================*/
void init_syms()
{
    free_syms();
    num_syms = 0;
}

/*========================================================================*//**
 * Free the symbol table
 *//*=========================================================================*/
void free_syms()
{
    sym_t* next = root;
    while (next)
    {
        cur = next;
        next = cur->next;
        free(cur);
    }
    root = NULL;
    cur = NULL;
}

/*=======================================================================*//**
 * Create a new symbol
 *
 * \param pass: assembly pass
 * \param id:   symbol's identifier
 * \param filename: name of the file in which the symbol is declared
 * \param line:     line in the file in which the symbol is declared
 * \param column:   column in the file in wich the symbol is declared
 *//*========================================================================*/
void sym_declare(int pass, char* id, char* filename, int line, int column)
{
    section_t* sect;
    sym_t* psym;
    sym_t* new;

    if (pass == GEN_PASS)
        return;

    sect = get_current_section();
    if (sect == NULL)
    {
        err(F, "symbol declaration before a section has been created");
        return;
    }

    psym = root;
    while (psym)
    {
        if (strcmp(psym->id, id) == 0)
        {
            err(E, "redefinition of '%s'", id);
            fprintf(stderr,
                "%s:%d:%d: %sprevious definition of '%s' was here\n",
                psym->filename, psym->line, psym->column, notestr, id
                );
            return;
        }
        psym = psym->next;
    }

    new = (sym_t*)mmalloc(sizeof(sym_t));

    if (root == NULL)
    {
        root = new;
        cur = root;
    }
    else
    {
        cur->next = new;
        cur = new;
    }

    cur->filename = (char*)mmalloc(strlen(filename) + 1);

    cur->sym_id = num_syms;
    strcpy(cur->id, id);
    cur->section_id = sect->id;
    cur->offset = sect->pc;
    cur->type = 0;
    strcpy(cur->filename, filename);
    cur->line = line;
    cur->column = column;
    cur->next = NULL;

    ++num_syms;
}

/*========================================================================*//**
 * Search for a symbol and return its absolute or relative address. If no
 * symbol is found, create a new symbol and mark it as extern. If the address
 * cannot be resolved, 0 is returned and relocation informations are created.
 *
 * \param pass: assembly pass
 * \param id: symbol's identifier
 * \param relative: non-zero means the symbol is requested by a relative jump
 * \return address of the symbol if absolute, distance if relative
 *
 * \todo handle symbol relocation
 *//*=========================================================================*/
int sym_request(int pass, char* id, int relative)
{
    sym_t* psym;
    section_t* cursect;
    section_t* targetsect;

    if (pass == READ_PASS)
        return 0;
    
    psym = root;
    while (psym)
    {
        if (strcmp(psym->id, id) == 0)
            break;
        psym = psym->next;
    }

    /* The symbol cannot be found: create it, mark it as imported and add
    a relocation information */
    if (psym == NULL)
    {
        /* Force symbol declaration by simulating the read pass */
        sym_declare(READ_PASS, id, " ", 0, 0);
        cur->type = _extern;
        /* Use offset+1 since all jump instructions are 1 byte long */
        add_reloc(cur->sym_id, cur->section_id, cur->offset+1, relative);
        if (relative)
            err(W, "relative jump to an external address");

        return 0;
    }

    cursect = get_current_section();
    
    /* Imported symbol: add a relocation information */
    if (psym->type == _extern)
    {
        add_reloc(psym->sym_id, cursect->id, cursect->pc+1, relative);
        if (relative)
            err(W, "relative jump to an external address");
        return 0;
    }

    if (relative)
    {
        if (psym->section_id != cursect->id)
        {
            err(W, "relative jump to a different section");
            add_reloc(psym->section_id, cursect->id, cursect->pc, relative);
            return 0;
        }
        else
        {
            int diff = psym->offset - (cursect->pc + 2);
            if (diff > 128 || diff < -127)
            {
                err(E, "relative jump to '%s' out of range", psym->id);
                return 0;
            }
            return diff;
        }
    }


    targetsect = get_section_by_id(psym->section_id);
    if (targetsect->type == org)
        return targetsect->offset + psym->offset;

    TODO("Relocation");

    return 0;
}

/*========================================================================*//**
 * Mark a symbol as global
 *
 * \param pass: assembly pass
 * \param id: symbol identifier
 *//*=========================================================================*/
void sym_set_global(int pass, char* id)
{
    sym_t* psym;
    if (pass == READ_PASS)
        return;

    psym = root;
    while (psym)
    {
        if (strcmp(psym->id, id) == 0)
            break;
        psym = psym->next;
    }
    
    if (psym == NULL)
    {
        err(E, "symbol '%s' declared global but not defined", id);
        return;
    }
    
    if (psym->type == _extern)
    {
        err(E, "symbol '%s' declared global but not defined", id);
        return;
    }
    
    if (psym->type == _global)
        err(W, "symbol '%s' declared global more than once", id);
    
    psym->type = _global;
}

/*========================================================================*//**
 * Write the symbol table to the output file
 *//*=========================================================================*/
void write_syms()
{
    block_header_t header;
    symbol_entry_t sym;
    if (num_syms == 0)
        return;

    header.type = symbols;
    header.num_entries = num_syms;
    write_block_header(&header);
    cur = root;
    while (cur)
    {
        sym.sym_id = cur->sym_id;
        strncpy((char*)sym.id, cur->id, MAX_ID_LEN + 1);
        sym.section_id = cur->section_id;
        sym.offset = cur->offset;
        sym.type = cur->type;
        write_symbol_entry(&sym);
        cur = cur->next;
    }
}

/**
 * \} Symbols
 * \} gbas
 */
