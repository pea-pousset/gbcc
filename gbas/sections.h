/**
 * \addtogroup gbas
 * \{
 * \addtogroup Sections
 * \{
 */

#ifndef SECTIONS_H
#define SECTIONS_H

#include <stdio.h>
#include "../common/objfile.h"

/** Describes a section entry in the sections list */
typedef struct section_s
{
    int            id;          /**< Section id */
    section_type_t type;        /**< Section type */
    int            offset;      /**< Absolute address or offset in the bank */
    int            bank;        /**< Bank number */
    int            datasize;    /**< Size of the section */
    int            pc;          /**< Program counter */
    struct section_s*     next; /**< Pointer to the next section in the list */
} section_t;

void       init_sections();
void       free_sections();
section_t* get_current_section();
section_t* get_section_by_id(int id);
void       add_section(int pass, section_type_t type, int address, int bank);
void       add_opcode(int pass, int iopcode, int val);
void       add_data(int pass, char c);

#endif

/**
 * \} Sections
 * \} gbas
 */
