/**
 * \addtogroup gbas
 * \{
 * \addtogroup Opcodes
 * \{
 */

#ifndef OPCODES_H
#define OPCODES_H

/**
 * Number of different keywords (including registers ) in the 'keywords'
 * table
 */
#define NUM_KEYWORDS        62

/** Number of entries in the 'opcodes' table */
#define NUM_OPCODES         500

/** Column of the first argument in the string of an opcode_t */
#define ARG1_COLUMN         5

/** Column of the first argument placed within brackets */
#define ARG_1_BRACKETED     7

#define ARG2_COLUMN         14
#define ARG_2_BRACKETED     16

#define MAX_OP_LEN          4

/**
 * Entry for an instruction in the lookup table
 */
typedef struct opcode_s
{
    const char* str;    /**< Mnemonic string */
    int         pre;    /**< 0xCB prefix */
    int         oc;     /**< Machine opcode */
    int         len;    /**< Length of the machine opcode, prefix included */
} opcode_t;

extern const char* keywords[NUM_KEYWORDS];
extern const opcode_t opcodes[NUM_OPCODES];

#endif

/**
 * \} Opcodes
 * \} gbas
 */
