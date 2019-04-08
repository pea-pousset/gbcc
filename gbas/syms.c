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
#include "commons.h"
#include "sections.h"
#include "outfile.h"

static sym_t* root = NULL;
static sym_t* cur = NULL;
static int num_syms;

void init_syms()
{
    cur = root;
    while (cur)
    {
        root = cur;
        cur = cur->next;
        free(root);
    }

    root = NULL;
    cur = NULL;
    num_syms = 0;
}

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

    strcpy(cur->id, id);
    cur->section_id = sect->id;
    cur->offset = sect->pc;
    cur->global = 0;
    cur->extern_ = 0;
    strcpy(cur->filename, filename);
    cur->line = line;
    cur->column = column;
    cur->next = NULL;

    ++num_syms;
}

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
        sym_declare(pass, id, " ", 0, 0);
        cur->extern_ = 1;
        /* ADD RELOCATION ########################## */
        if (relative)
            err(W, "relative jump to an external address");

        return 0;
    }

    /* Imported symbol: add a relocation information */
    if (psym->extern_)
    {
        /* ADD RELOCATION ########################## */
        return 0;
    }

    cursect = get_current_section();
    if (relative)
    {
        if (psym->section_id != cursect->id)
        {
            err(W, "relative jump to a different section");
            /* RELOCATION ############################# */
            return 0;
        }
        else
        {
            int diff = psym->offset - (cursect->pc + 2);
            if (diff > 128 || diff < -127)
            {
                err(E, "relative jump out of range");
                return 0;
            }
            return diff;
        }
    }


    targetsect = get_section_id(psym->section_id);
    if (targetsect->type == org)
        return targetsect->address + psym->offset;

                /* RELOCATION ############################# */

    return 0;
}

void write_syms()
{
    if (num_syms == 0)
        return;

    write_block_header(symbols, num_syms);
    cur = root;
    while (cur)
    {
        write_data(cur->id, MAX_ID_LEN + 1);
        write_int32(cur->section_id);
        write_int16(cur->offset);
        write_int8(cur->global);
        write_int8(cur->extern_);
        cur = cur->next;
    }
}

/**
 * \} Symbols
 * \} gbas
 */
