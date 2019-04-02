#ifndef AST_H
#define AST_H

typedef void* node_t;

void ast_init();
void ast_free();
void ast_add_node(node_t n);
void ast_inc_scope(int scope_id);
void ast_dec_scope();
node_t function(const char* name);
node_t constant(int value);
node_t identifier(const char* name);
node_t unop(int id, node_t right);
node_t binop(int id, node_t left, node_t right);
node_t jump(int id, node_t arg);

#include <stdio.h>
void ast_dump(FILE* f);

#endif
