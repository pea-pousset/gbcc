/**
 * \addtogroup gbcc
 * \{
 * \addtogroup ast
 * \{
 */
#ifndef ASTCOMMONS_H
#define ASTCOMMONS_H

#include "lexer.h"

#define LEFTOP  0
#define RIGHTOP 1

/** Describes a node type */
enum node_type_e
{
    STATEMENT_LIST = 1,
    FUNCTION,
    BINOP,
    UNOP,
    ASSIGN
};

/** Describes a node of the abstract syntax tree */
typedef struct _node_s
{
    int                 id;             /**< Token id */
    enum node_type_e    type;           /**< Node type */
    unsigned int        num_children;   /**< Number of child nodes */
    struct _node_s*     parent;         /**< Parent node */
    struct _node_s**    children;       /**< Child nodes */
    int     num_value;                  /**< Contains the value of a constant
                                         or the scope ID of a block node */
    char    identifier[MAX_ID_LEN + 1]; /**< Contains a null-terminated
                                         indentifier string */
} _node_t;


_node_t* ast_get_tree();

#endif

/**
 * \} ast
 * \} gbcc
 */
