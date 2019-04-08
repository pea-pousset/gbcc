/**
 * \addtogroup gbcc
 * \{
 * \addtogroup Lexer
 * \}
 */

#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#define MAX_ID_LEN  31

/** Token IDs. Single character tokens return their own ascii value,
 * including EOF */
enum token_id
{
    BYTE = 128,
    ELSE,
    IF,
    RETURN,
    VOID,
    WORD,

    IDENTIFIER,
    CONSTANT,

    EQ_OP,      /* == */
    NE_OP,      /* != */
    LE_OP,      /* <= */
    GE_OP,      /* >= */

    NEGATIVE,   /* Unary + , not generated by the lexer but used as an id by the parser */
    POSITIVE    /* Unary - */
};

typedef struct
{
    int          id;
    unsigned int line;
    unsigned int column;
    unsigned int constant;
    int          _unsigned;
    int          _word;
    char         string[MAX_ID_LEN + 1];
} token_t;

void        lexer_init(FILE* infile);
token_t*    get_token();
const char* token_name(int id);
void        lexer_backtrace_prepare();
void        lexer_backtrace();
void        token_copy(token_t* from, token_t* to);

#endif

/**
 * \} Lexer
 * \} gbcc
 */
