#include "ast.h"
#include "astcommons.h"

#include <stdlib.h>
#include <string.h>

#include "../common/errors.h"
#include "../common/utils.h"
#include "lexer.h"

_node_t*        root = NULL;    /**< Root of the AST */
static _node_t* node = NULL;    /**< The current node */

static _node_t* create_node();
static void     add_child(_node_t* to, _node_t* child);
static void     free_node(_node_t* n);


static void     ast_dump_node(FILE* f, _node_t* n, int indent);

/*========================================================================*//**
 * Initialize the abstract syntax tree
 *//*=========================================================================*/
void ast_init()
{
    ast_free();
    root = create_node();
    root->id = STATEMENT_LIST;
    root->num_value = 0;
    node = root;
}

/*========================================================================*//**
 * Clear the abstract syntax tree
 *//*=========================================================================*/
void ast_free()
{
    if (root)
    {
        free_node(root);
        root = NULL;
        node = NULL;
    }
}

/*========================================================================*//**
 * Returns the whole abstract syntax tree
 *
 * \return Root of the AST
 *//*=========================================================================*/
_node_t* ast_get_tree()
{
    return root;
}

/*========================================================================*//**
 * Adds a child node to the abstract syntax tree to the current node
 *
 * \param n: The node to add
 *//*=========================================================================*/
void ast_add_node(node_t n)
{
    add_child(node, (_node_t*)n);
}

/*========================================================================*//**
 * Increases the current scope by adding a statement block node
 *
 * \param scopde_id: The new scope id
 *//*=========================================================================*/
void ast_inc_scope(int scope_id)
{
    _node_t* n = create_node();
    n->type = STATEMENT_LIST;
    n->num_value = scope_id;
    add_child(node, n);
    node = n;
}

/*========================================================================*//**
 * Decreases the current scope
 *//*=========================================================================*/
void ast_dec_scope()
{
    if (node->parent)
        node = node->parent;
}

/*========================================================================*//**
 * Creates a new 'function' node
 *
 * \param name: The function's name
 *//*=========================================================================*/
node_t function(const char* name)
{
    _node_t* new = create_node();
    new->type = FUNCTION;
    strcpy(new->identifier, name);
    return (node_t)new;
}

/*========================================================================*//**
 * Creates a new node containing a numeric constant
 *
 * \param value: constant value
 *//*=========================================================================*/
node_t constant(int value)
{
    _node_t* new = create_node();
    new->id = CONSTANT;
    new->num_value = value;
    return (node_t)new;
}

/*========================================================================*//**
 * Creates a new 'identifier' node
 *
 * \param name: identifier string
 *//*=========================================================================*/
node_t identifier(const char* name)
{
    _node_t* new = create_node();
    new->id = IDENTIFIER;
    strcpy(new->identifier, name);
    return (node_t)new;
}

/*========================================================================*//**
 * Creates a new 'unary operator' node
 *
 * \param id: Token id
 * \param right: rvalue
 *//*=========================================================================*/
node_t unop(int id, node_t right)
{
    _node_t* new = create_node();
    if (id == '+')
        new->id = POSITIVE;
    else if (id == '-')
        new->id = NEGATIVE;
    else
        new->id = id;
    new->type = UNOP;
    add_child(new, (_node_t*)right);
    return (node_t)new;
}

/*========================================================================*//**
 * Creates a new 'binary operator' node
 *
 * \param id: Token id
 * \param left: lvalue
 * \param right: rvalue
 *//*=========================================================================*/
node_t binop(int id, node_t left, node_t right)
{
    _node_t* new = create_node();
    new->id = id;
    if (id == '=')
        new->type = ASSIGN;
    else
        new->type = BINOP;
    add_child(new, (_node_t*)left);
    add_child(new, (_node_t*)right);
    return (node_t)new;
}

/*========================================================================*//**
 * Creates a new 'jump statement'. 'arg' can be NULL, an
 * expression for 'return' or an identifier for 'goto'.
 *//*=========================================================================*/
node_t jump(int id, node_t arg)
{
    _node_t* new = create_node();
    new->id = id;
    if (arg != NULL)
        add_child(new, (_node_t*)arg);
    return (node_t)new;
}






_node_t* create_node()
{
    _node_t* new = (_node_t*)mmalloc(sizeof(_node_t));

    new->type = 0;
    new->num_children = 0;
    new->parent = NULL;
    new->children = NULL;
    return new;
}

void add_child(_node_t* to, _node_t* child)
{
    _node_t** new;
    to->num_children++;
    new =
        (_node_t**)mrealloc(to->children, sizeof(_node_t*) * to->num_children);

    to->children = new;
    to->children[to->num_children - 1] = child;
    child->parent = to;
}

void free_node(_node_t* n)
{
    unsigned int i;
    for (i = 0; i < n->num_children; i++)
    {
        if (n->children[i])
            free_node(n->children[i]);
    }

    if (n->children)
        free(n->children);

    free(n);
}



void ast_dump(FILE* f)
{
    fprintf(f, ";********************************  AST *****************************************\n");
    ast_dump_node(f, root, -1);
    fprintf(f, ";*******************************************************************************\n\n\n");
}

void ast_dump_node(FILE* f, _node_t* n, int indent)
{
    unsigned int i;
    if (n != root)
    {
        int ind = indent;
        fprintf(f, "; ");
        while (ind--)
            fprintf(f, "    ");

        if (n->type == STATEMENT_LIST)
        {
            fprintf(f, "SCOPE %d\n", n->num_value);
        }
        else if (n->type == FUNCTION)
        {
            fprintf(f, "FUNCTION %s\n", n->identifier);
        }
        else if (n->type == ASSIGN)
        {
            fprintf(f, "assign\n");
        }
        else if (n->id == IDENTIFIER)
        {
            fprintf(f, "%s\n", n->identifier);
        }
        else if (n->id == CONSTANT)
        {
            fprintf(f, "%d\n", n->num_value);
        }
        else if (n->id == POSITIVE)
        {
            fprintf(f, "+ve\n");
        }
        else if (n->id == NEGATIVE)
        {
            fprintf(f, "-ve\n");
        }
        else
            fprintf(f, "%s\n", token_name(n->id));
    }

    for (i = 0; i < n->num_children; i++)
    {
        if (n->children[i])
            ast_dump_node(f, n->children[i], indent + 1);
    }
}
