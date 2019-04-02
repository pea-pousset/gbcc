#ifndef ASTCOMMONS_H
#define ASTCOMMONS_H

#include "lexer.h"

#define LEFTOP  0
#define RIGHTOP 1

enum node_type_e
{
    STATEMENT_LIST = 1,
    FUNCTION,
    BINOP,
    UNOP,
    ASSIGN
};

typedef struct _node_s _node_t;
struct _node_s
{
    int                 id;             /**< token id */
    enum node_type_e    type;
    unsigned int        num_children;   /**< number of child nodes */
    _node_t*            parent;
    _node_t**           children;

    /** Contains the value of a constant or the scope ID of a
    block node */
    int     num_value;
    char    identifier[MAX_ID_LEN + 1];
};


_node_t* ast_get_tree();

#endif
