/**
 * \addtogroup gbcc
 * \{
 * \defgroup code_generation Code generation
 * \addtogroup code_generation
 * \{
 */

#include "codegen.h"
#include <stdio.h>

#include "../common/errors.h"
#include "lexer.h"
#include "astcommons.h"
#include "syms.h"

static FILE* f;

void export_syms(symtbl_t* tbl);
void export_funcs(symtbl_t* tbl);
void gen_block(_node_t* node);
void gen_expr(_node_t* node);
void gen_assign(_node_t* node);
void gen_jump(_node_t* node);
sym_t* gen_lvalue(_node_t* node);

void codegen(FILE* outfile)
{
    _node_t* tree = ast_get_tree();
    if (!tree)
        return;

    f = outfile;

    fprintf(f, ".org $C000 ; TMP BS\n");
    export_syms(syms_get_table());

    fprintf(f, ".org $200  ; TMP BS\n");
    export_funcs(syms_get_table());
    gen_block(tree);
}

void export_syms(symtbl_t* tbl)
{
    unsigned int i;

    for (i = 0; i < tbl->num_syms; ++i)
    {
        if (tbl->syms[i]->function)
            continue;
        if (tbl->scope_id == 0)
            fprintf(f, ".global %s\n", tbl->syms[i]->id);
    }

    for (i = 0; i < tbl->num_syms; ++i)
    {
        if (tbl->syms[i]->function)
            continue;

        fprintf(f, "%s:", tbl->syms[i]->id);
        fprintf(f, " %s\n", tbl->syms[i]->type_id == WORD ? ".word" : ".byte");
    }

    for (i = 0; i < tbl->num_children; i++)
        export_syms(tbl->children[i]);
}

void export_funcs(symtbl_t* tbl)
{
    unsigned int i;
    for (i = 0; i < tbl->num_syms; ++i)
    {
        if (!tbl->syms[i]->function)
            continue;

        fprintf(f, ".global %s\n", tbl->syms[i]->id);
    }
}

void gen_block(_node_t* node)
{
    unsigned int i;
    _node_t* child;

    syms_set_cur_scope(node->num_value);

    for (i = 0; i < node->num_children; i++)
    {
        child = node->children[i];
        if (child->type == STATEMENT_LIST)
        {
            gen_block(child);
        }
        else if (child->type == FUNCTION)
        {
            fprintf(f, "; FUNCTION\n");
            fprintf(f, "%s:\n", child->identifier);
        }
        else if (child->type == ASSIGN)
        {
            gen_assign(child);
        }
        else if (child->id == CONSTANT || child->id == IDENTIFIER ||
                 child->type == BINOP || child->type == UNOP)
        {
            gen_expr(child);
            fprintf(f, "        call print_a   ; DEBUG\n");
        }
        else if (child->id == RETURN)
        {
            gen_jump(child);
        }
        else
        {
            ccerr(F, "unhandled node");
        }
    }

    syms_dec_scope();
}

void gen_expr(_node_t* node)
{
    if (node->id == IDENTIFIER)
    {
        TODO("check existing ");
        fprintf(f, "        ld      bc, %s\n", node->identifier);
        fprintf(f, "        ld      a, [bc]\n"); /* 8-bits only */
    }
    else if (node->id == CONSTANT)
    {
        fprintf(f, "; load constant\n");
        if (!node->num_value)
            fprintf(f, "        xor     a\n");
        else
            fprintf(f, "        ld      a, $%02X\n", node->num_value);
    }
    else if (node->type == UNOP)
    {
        gen_expr(node->children[0]);

        switch(node->id)
        {
        case POSITIVE:
            break;

        case NEGATIVE:
            fprintf(f, "; negative (8 bits)\n");
            fprintf(f, "        cpl\n");
            fprintf(f, "        inc     a\n");
            break;
        }
    }
    else if (node->type == BINOP)
    {
        gen_expr(node->children[LEFTOP]);
        fprintf(f, "        push    af\n");
        gen_expr(node->children[RIGHTOP]);
        switch(node->id)
        {

        case '+':
            fprintf(f, "; addition \n");
            fprintf(f, "        pop     de\n");
            fprintf(f, "        add     a, d\n");
            break;

        case '-':
            fprintf(f, "; substraction \n");
            fprintf(f, "        ld      d, a\n");
            fprintf(f, "        pop     af\n");
            fprintf(f, "        sub     a, d\n");
            break;

        case '*':
            fprintf(f, "; multiplication \n");
            fprintf(f, "        pop     de\n");
            fprintf(f, "        ld      e, d\n");
            fprintf(f, "        ld      h, a\n");
            fprintf(f, "        call    ___mul_8_\n");
            fprintf(f, "        ld      a, l\n");
            break;

        case '/':
            fprintf(f, "; division \n");
            fprintf(f, "        pop     de\n");
            fprintf(f, "        ld      e, a\n");
            fprintf(f, "        call    ___div_u8_\n");
            fprintf(f, "        ld      a, d\n");
            break;

        case '%':
            fprintf(f, "; modulo \n");
            fprintf(f, "        pop     de\n");
            fprintf(f, "        ld      e, a\n");
            fprintf(f, "        call    ___div_u8_\n");
            break;
        }
    }
    else if (node->type == ASSIGN)
    {
        gen_assign(node);
    }
}

void gen_assign(_node_t* node)
{
    sym_t* s;
    TODO("assign word");
    fprintf(f, "; assign { \n");
    gen_expr(node->children[RIGHTOP]);  /* Load byte into A or word into HL */

    s = gen_lvalue(node->children[LEFTOP]); /* Load ptr into DE */
    if (s->type_id == BYTE)
    {
        fprintf(f, "        ld      [de], a\n");
    }
    else
    {
        fprintf(f, "        ld      [de], l\n");
        fprintf(f, "        inc     de\n");
        fprintf(f, "        ld      [de], h\n");
    }

    fprintf(f, "; } assign \n");
}

void gen_jump(_node_t* node)
{
    if (node->id == RETURN)
    {
        fprintf(f, "; *** return ***\n");
        if (node->num_children)
            gen_expr(node->children[0]);
        fprintf(f, "        ret\n");
    }
}

sym_t* gen_lvalue(_node_t* node)
{
    sym_t* s = get_sym(node->identifier);
    TODO("lvalue type check");
    assert(s != NULL);
    fprintf(f, "        ld      de, %s\n", node->identifier);
    return s;
}

/**
 * \} code_generation
 * \} gbcc
 */
