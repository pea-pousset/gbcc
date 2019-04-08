/**
 * \addtogroup gbcc
 * \{
 * \defgroup syms Symbols table
 * \addtogroup syms
 * \{
 */

#include "syms.h"

#include <stdlib.h>
#include <string.h>

#include "../common/errors.h"
#include "../common/utils.h"

static symtbl_t* find_scope_id(symtbl_t* s, int scope_id);
static void      free_symtbl(symtbl_t* s);

static int       scope_id;      /**< ++ each time a new scope is created */
static int       cur_scope_id;  /**< Current scope id, 0 meaning global scope */
static symtbl_t* root = NULL;   /**< Root of the symbol table */
static symtbl_t* scope = NULL;  /**< Current scope's symbol table */

/*========================================================================*//**
 * Initialize the symbol table
 *//*=========================================================================*/
void syms_init()
{
    syms_free();
    root = (symtbl_t*)mmalloc(sizeof(symtbl_t));
    root->scope_id = 0;
    root->num_syms = 0;
    root->num_children = 0;
    root->parent = NULL;
    root->children = NULL;
    root->syms = NULL;

    scope_id = 0;
    cur_scope_id = 0;

    scope = root;
}

/*========================================================================*//**
 * Clear the symbol table
 *//*=========================================================================*/
void syms_free()
{
    if (root)
    {
        free_symtbl(root);
        root = NULL;
        scope = NULL;
    }
}

/*========================================================================*//**
 * Set the current table given a scope id
 *//*=========================================================================*/
void syms_set_cur_scope(int scope_id)
{
    scope = find_scope_id(root, scope_id);
}

/*========================================================================*//**
 * Return the whole symbol table
 *//*=========================================================================*/
symtbl_t* syms_get_table()
{
    return root;
}

/*========================================================================*//**
 * Create a new symbol
 *//*=========================================================================*/
sym_t* create_sym()
{
    sym_t* s = (sym_t*)mmalloc(sizeof(sym_t));
    s->id[0] = 0;
    s->type_id = 0;
    s->function = 0;

    return s;
}

/*========================================================================*//**
 * Return the symbol associated to the identifier 'id' if it's accessible from
 * the current scope, otherwise return NULL
 *//*=========================================================================*/
sym_t* get_sym(const char *id)
{
    unsigned int i;
    symtbl_t* scp = scope;
    do
    {
        for (i = 0; i < scp->num_syms; i++)
        {
            if (strcmp(scp->syms[i]->id, id) == 0)
                return scp->syms[i];
        }
        scp = scp->parent;
    } while (scp != NULL);

    return NULL;
}

/*========================================================================*//**
 * Add a symbol to the table or destroy it if the declaration was
 * empty
 *//*=========================================================================*/
void syms_add(sym_t* s)
{
    int i;
    /* empty declaration like 'static byte;' */
    if (s->id[0] == 0)
    {
        free(s);
        return;
    }

    /* set default type */
    if (!s->type_id)
        s->type_id = BYTE;

    /* check for redefinition
    for (i = 0; i < scope->num_syms; ++i)
    {
        if (strcmp(s->id, scope->syms[i]->id) == 0)
        {
            err(E, "redefinition of '%s'", s->id);
            return;
        }
    }
    */

    scope->num_syms++;
    scope->syms = (sym_t**)mrealloc(scope->syms,
                                   sizeof(sym_t*) * scope->num_syms);
    scope->syms[scope->num_syms - 1] = s;


}

/*========================================================================*//**
 * Create a new scope, use it as the current scope and return its ID
 *//*=========================================================================*/
int syms_inc_scope()
{
    symtbl_t* new;
    scope->num_children++;
    scope->children = (symtbl_t**)mrealloc(scope->children,
                                           sizeof(symtbl_t*) * scope->num_children);
    new = (symtbl_t*)mmalloc(sizeof(symtbl_t));

    ++scope_id;
    cur_scope_id = scope_id;

    new->scope_id = scope_id;
    new->parent = scope;
    new->num_syms = 0;
    new->num_children = 0;
    new->syms = NULL;
    new->children = NULL;

    scope->children[scope->num_children - 1] = new;
    scope = new;

    return cur_scope_id;
}

/*========================================================================*//**
 * After a call to syms_inc_scope() or syms_set_cur_scope(), go back
 * to the parent scope.
 *//*=========================================================================*/
void syms_dec_scope()
{
    if (scope->parent)
    {
        scope = scope->parent;
        cur_scope_id = scope->scope_id;
    }
}

/*========================================================================*//**
 * Find the symbol table for a given scope id among the children
 * of the table 's'. If no table has been found, return NULL.
 *//*=========================================================================*/
symtbl_t* find_scope_id(symtbl_t* s, int scope_id)
{
    unsigned int i;
    symtbl_t* ret;
    if (s->scope_id == scope_id)
        return s;

    for (i = 0; i < s->num_children; i++)
    {
        if (s->children[i]->scope_id == scope_id)
            return s->children[i];

        ret = find_scope_id(s->children[i], scope_id);
        if (ret)
            return ret;
    }
    return NULL;
}

/*========================================================================*//**
 * Recursively free the symbol table
 *//*=========================================================================*/
void free_symtbl(symtbl_t* s)
{
    unsigned int i;
    if (s->children)
    {
        for (i = 0; i < s->num_children; i++)
            free_symtbl(s->children[i]);
        free (s->children);
    }

    if (s->syms)
    {
        for (i = 0; i < s->num_syms; i++)
            free(s->syms[i]);
        free(s->syms);
    }

    free(s);
}




void syms_dump_e(FILE* f, symtbl_t* t)
{
    unsigned int i;
    if (t->num_syms)
        fprintf(f, ";-------------------------------------------------------------------------------\n");
    for (i = 0; i < t->num_syms; i++)
    {
        sym_t* s = t->syms[i];
        fprintf(f, "; %-31s %-9s %-5s %-6d\n", s->id, token_name(s->type_id), s->function ? "YES" : "", t->scope_id);
    }

    for (i = 0; i < t->num_children; i++)
    {
        if (t->children[i])
            syms_dump_e(f, t->children[i]);
    }
}

void syms_dump(FILE* f)
{
    fprintf(f, ";**************************** SYMBOL TABLE *************************************\n;\n");
    fprintf(f, "; %-31s %-9s %-5s %-6s\n", "NAME", "TYPE", "FUNC", "SCOPE");
    syms_dump_e(f, root);
    fprintf(f, ";*******************************************************************************\n\n\n");
}

/**
 * \} syms
 * \} gbcc
 */
