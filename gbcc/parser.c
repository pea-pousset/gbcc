/*========================================================================*//**
 * Recursive descent parser
 * \defgroup parser
 *//*=========================================================================*/
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/errors.h"
#include "../common/options.h"
#include "lexer.h"
#include "ast.h"
#include "syms.h"
#include "codegen.h"

static token_t *t;      /**< The current token */
static int     last_id; /**< Last token type */

static void    next();
static int     sync(const char* accepted);
static int     expect(int id);
static int     accept(int id);

static int     is_declaration();

static node_t  primary_expression();
static node_t  postfix_expression();
/* argument_expression_list */
static node_t  unary_expression();
static int     unary_operator();
static node_t  cast_expression();
static node_t  multiplicative_expression();
static node_t  additive_expression();
static node_t  shift_expression();
static node_t  relational_expression();
static node_t  equality_expression();
static node_t  and_expression();
static node_t  exclusive_or_expression();
static node_t  inclusive_or_expression();
static node_t  logical_and_expression();
static node_t  logical_or_expression();
static node_t  conditional_expression();
static node_t  assignment_expression();
/* assignment_operator */
static node_t  expression();
/* constant_expression */

static int     declaration();
static int     declaration_specifiers(sym_t* s);
static int     init_declarator_list(sym_t* s);
static int     init_declarator(sym_t* s);
static int     type_specifier(sym_t* s);


static int     declarator(sym_t* s);
static int     direct_declarator(sym_t* s);

static int     initializer(sym_t* s);


static int     statement();
/* labeled_statement */
static int     compound_statement();
static int     declaration_list();
static int     statement_list();
static int     expression_statement();
static int     jump_statement();

static int     function_definition();

int parse(FILE* infile, FILE* outfile)
{
    int parse_error = errors();
    ast_init();
    syms_init();
    lexer_init(infile);

    t = get_token();
    last_id = ' ';

    /* Translation unit */
    while (t->id != EOF)
    {
        if (function_definition())
            ;
        else if (declaration())
            ;
        else
        {
            if (last_id == ' ')
                err(E, "expected statement");   /* Blank file */
            else
                err(E, "expected statement after %s", token_name(last_id));

            if (t->id == '{')
            {
                if (!sync("}"))
                    next();
            }
            else
                next();
        }
    }

    syms_dump(outfile);
    ast_dump(outfile);
    codegen(outfile);
    ast_free();
    syms_free();
    return errors() == parse_error;
}

/*========================================================================*//**
 * Get the next token from the lexer
 *//*=========================================================================*/
void next()
{
    last_id = t->id;
    t = get_token();
    /* Update the error message's line and column */
    eline = t->line;
    ecolumn = t->column;
}

int sync(const char* accepted)
{
    int loop = 1;
    const char* p;
#ifndef NDEBUG
    printf("Trying to resync with one of : ");
    p = accepted;
    while (*p)
    {
        printf("'%c'", *p);
        ++p;
        if (*p)
            printf(", ");
    }
    printf("\n\tLast token : %s\n\tCurrent : %s\n", token_name(last_id), token_name(t->id));
#endif
    while (loop)
    {
        p = accepted;
        while (*p && t->id != EOF && *p != t->id)
            ++p;

        if (*p)
            loop = 0;
        else
            next();
    }
#ifndef NDEBUG
    printf("\tResult: %s found at %d:%d\n", token_name(t->id), t->line, t->column);
#endif
    if (t->id != EOF)
    {
        next();
        return 1;
    }
    return 0;
}

/*========================================================================*//**
 * If the current token type is 'id', fetch the next token and
 * return 1, otherwise return 0.
 *//*=========================================================================*/
int accept(int id)
{
    if (t->id == id)
    {
        next();
        return 1;
    }
    return 0;
}

/*========================================================================*//**
 * If the current token type is 'id', fetch the next token and
 * return 1, otherwise generate an error and return 0.
 *//*=========================================================================*/
int expect(int id)
{
    if (accept(id))
        return 1;
    err(E, "expected %s after %s", token_name(id), token_name(last_id));
    return 0;
}

/*========================================================================*//**
 * Tell whether the current token starts a new declaration or not
 *//*=========================================================================*/
int is_declaration()
{
    switch (t->id)
    {
        case BYTE: return 1;
        case VOID: return 1;
        case WORD: return 1;

        /* case const, struct, static etc */
    }

    /*
    if (ast_get_scope() == 0 && t->id == IDENTIFIER)    USELESS IN NON K&R FUNCTION DEFINITION
        return 1; */

    return 0;
}

/*========================================================================*//**
 * Try to parse a primary expression. If it succeeds, return a new
 * AST node, otherwise return NULL.
 *//*=========================================================================*/
node_t primary_expression()
{
    node_t n = NULL;

    if (t->id == IDENTIFIER)
    {
        n = identifier(t->string);
        next();
        return n;
    }
    else if (t->id == CONSTANT)
    {
        n = constant(t->constant);
        next();
        return n;
    }
    else if (accept('('))
    {
        if ((n = expression()))
        {
            if (expect(')'))
                return n;
            else
                sync(")");
        }
        else
        {
            err(E, "expected expression after %s", token_name(last_id));
            sync(")");
        }
    }

    return NULL;
}

node_t postfix_expression()
{
    return primary_expression();
}

node_t unary_expression()
{
    node_t n = NULL;
    if ((n = postfix_expression()))
    {
        return n;
    }
    else if (unary_operator())
    {
        int op = t->id;
        node_t right = NULL;
        next();
        if (!(right = cast_expression()))
        {
            err(E, "expected expression after %s", token_name(last_id));
            return NULL;
        }

        n = unop(op, right);

        return n;
    }

    return NULL;
}

/*========================================================================*//**
 * If the current token is a unary operator, return 1, otherwise
 * return 0.
 *//*=========================================================================*/
int unary_operator()
{
    if (t->id == '+')
        return 1;
    else if (t->id == '-')
        return 1;
    return 0;
}

node_t cast_expression()
{
    return unary_expression();
}

node_t multiplicative_expression()
{
    node_t n = NULL;
    if ((n = cast_expression()))
    {
        while (t->id == '*' || t->id == '/' || t->id == '%')
        {
            node_t right = NULL;
            int op = t->id;
            next();
            if (!(right = cast_expression()))
            {
                err(E, "expected expression after %s ", token_name(last_id));
                return NULL;
            }

            n = binop(op, n, right);
        }
        return n;
    }
    return NULL;
}

node_t additive_expression()
{
    node_t n = NULL;
    if ((n = multiplicative_expression()))
    {
        while (t->id == '+' || t->id == '-')
        {
            node_t right = NULL;
            int op = t->id;
            next();

            if (!(right = multiplicative_expression()))
            {
                err(E, "expected expression after %s", token_name(t->id));
                return NULL;
            }
            n = binop(op, n, right);
        }
        return n;
    }

    return NULL;
}

node_t shift_expression()
{
    return additive_expression();
}

node_t relational_expression()
{
    return shift_expression();
}

node_t equality_expression()
{
    return relational_expression();
}

node_t and_expression()
{
    return equality_expression();
}

node_t exclusive_or_expression()
{
    return and_expression();
}

node_t inclusive_or_expression()
{
    return exclusive_or_expression();
}

node_t logical_and_expression()
{
    return inclusive_or_expression();
}

node_t logical_or_expression()
{
    return logical_and_expression();
}

node_t conditional_expression()
{
    return logical_or_expression();
}

node_t assignment_expression()
{
    return conditional_expression();
}

node_t expression()
{
    node_t n = NULL;
    if ((n = assignment_expression()))
        return n;

    return NULL;
}

int declaration()
{
    sym_t* s = create_sym();

    if (declaration_specifiers(s))
    {
        if (t->id == ';')
        {
            switch(last_id)
            {
                case BYTE:
                case WORD:
                case VOID:
                    err(W, "useless type name in empty declaration");
                    break;

                default:
                    err(W, "empty declaration");
            }
        }

        init_declarator_list(s);

        if (s->type_id == VOID && !s->function && s->id[0])
            err(E, "variable or field '%s' declared void", s->id);

        if (t->id == ';') /* Do not use expect to not fuck up eline */
        {
            syms_add(s);
            next();
            return 1;
        }
        else
            expect(';'); /* Use expect to generate the error message */

        sync(";");
    }

    free(s);
    return 0;
}


int declaration_specifiers(sym_t* s)
{
    if (type_specifier(s))
    {
        while (declaration_specifiers(s))
            ;
        return 1;
    }

    return 0;
}

int init_declarator_list(sym_t* s)
{
    if (init_declarator(s))
    {
        while (t->id == ',')
        {
            sym_t* s2 = create_sym();
            strcpy(s2->id, s->id);
            s2->type_id = s->type_id;
            s->id[0] = 0;
            syms_add(s2);

            next();

            if (!init_declarator(s))
                return 0;   /* ?????????????? */
        }
        return 1;
    }

    return 0;
}

int init_declarator(sym_t* s)
{
    if (declarator(s))
    {
        if (accept('='))
        {
            if (initializer(s))
            {
                if (s->function)
                    err(E, "function '%s' declared like a variable", s->id);
            }
            else
                err(E, "expected expression after '='");
        }
        return 1;
    }
    return 0;
}

int type_specifier(sym_t* s)
{
    if (accept(BYTE))
    {
        if (s->type_id )
            err(E, "two or more data types in declaration specifiers");
        else
            s->type_id = BYTE;
        return 1;
    }
    else if (accept(VOID))
    {
        if (s->type_id)
            err(E, "two or more data types in declaration specifiers");
        else
            s->type_id = VOID;
        return 1;
    }
    else if (accept(WORD))
    {
        if (s->type_id )
            err(E, "two or more data types in declaration specifiers");
        else
            s->type_id = WORD;
        return 1;
    }

    return 0;
}

int declarator(sym_t* s)
{
    if (direct_declarator(s))
        return 1;

    return 0;
}

int direct_declarator(sym_t* s)
{
    if (t->id == IDENTIFIER)
    {
        strcpy(s->id, t->string);
        next();

        while (/* t->id == '[' || */ t->id == '(' )
        {
            /* if (t->id == '[')
            {
            }
            else */ if (accept('('))
            {

                if (s->function)
                {
                    err(E, "'%s' declared as a function returning a function", s->id);
                    sync(")");
                }
                else
                {
                    s->function = 1;
                    if (expect(')'))
                        ;
                    else
                        sync(")");
                }
            }

        }

        return 1;
    }
    /*
    else if (t->id == '(')
    {
        declarator etc
        if (t->id == '['
    }

    */
    return 0;
}

int initializer(sym_t* s)
{
    node_t right = NULL;
    if ((right = assignment_expression()))
    {
        node_t n = binop('=', identifier(s->id), right);
        ast_add_node(n);
        return 1;
    }

    return 0;
}

int statement()
{
    if (compound_statement())
        return 1;
    else if (jump_statement())
        return 1;
    /* expression_statement() has to be the last in the list, as in case of
       error it will try to resync on a ";" and yield true if synced, making the
       statement an empty statement and allowing the parsing to continue */
    else if (expression_statement())
        return 1;

    return 0;
}

int compound_statement()
{
    if (accept('{'))
    {
        ast_inc_scope(syms_inc_scope());

        if (accept('}'))
        {
            ast_dec_scope();
            syms_dec_scope();
            return 1;
        }
        else if (declaration_list())
        {
            if (statement_list())
            {
                if (expect('}'))
                {
                    ast_dec_scope();
                    syms_dec_scope();
                    return 1;
                }
            }
            else if (expect('}'))
            {
                ast_dec_scope();
                syms_dec_scope();
                return 1;
            }
        }
        else if (statement_list())
        {
            if (expect('}'))
            {
                ast_dec_scope();
                syms_dec_scope();
                return 1;
            }
        }
        else
        {
            err(E, "expected '}' or statement after %s", token_name(last_id));
        }


        if (sync("}"))
        {
            ast_dec_scope();
            syms_dec_scope();
        }

        return 1;
    }
    return 0;
}

int declaration_list()
{
    if (is_declaration())
    {
        declaration();
        while (is_declaration())
        {
            declaration();
        }
        return 1;
    }
    return 0;
}

int statement_list()
{
    if (statement())
    {
        while (statement())
            ;
        return 1;
    }

    return 0;
}

int expression_statement()
{
    node_t n;
    if (accept(';'))
        return 1;
    else if ((n = expression()))
    {
        if (expect(';'))
        {
            ast_add_node(n);
            return 1;
        }
        else
            return sync(";");
    }
    return 0;
}

int jump_statement()
{
    node_t n = NULL;
    if (accept(RETURN))
    {
        n = expression();
        TODO("Check return type ?");
        if (expect(';'))
        {
            ast_add_node(jump(RETURN, n));
            return 1;
        }
        else
            return sync(";");
    }
    return 0;
}

int function_definition()
{
    sym_t* s = create_sym();
    token_t bt;
    int blast_id;

    backtrace_prepare();
    token_copy(t, &bt);
    blast_id = last_id;

    if (declarator(s))
    {
        if (!s->function)
        {
            backtrace();
            token_copy(&bt, t);
            last_id = blast_id;
            free(s);
            return 0;
        }

        if (t->id == '{')
        {
            syms_add(s);
            ast_add_node(function(s->id));
            compound_statement();
            return 1;
        }
    }
    else if (declaration_specifiers(s))
    {
        if (declarator(s))
        {
            if (!s->function)
            {
                backtrace();
                token_copy(&bt, t);
                last_id = blast_id;
                free(s);
                return 0;
            }

            if (t->id == '{')
            {
                syms_add(s);
                ast_add_node(function(s->id));
                compound_statement();
                return 1;
            }
        }
    }

    backtrace();
    token_copy(&bt, t);
    last_id = blast_id;
    free(s);
    return 0;
}
