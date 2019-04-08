/**
 * \addtogroup gbas
 * \{
 * \addtogroup Sections
 * \{
 */

#ifndef SECTIONS_H
#define SECTIONS_H

#include <stdio.h>

typedef enum
{
    org
} section_type_t;

typedef struct section_s
{
    int            id;
    section_type_t type;
    int            address;
    int            datasize;

    int            pc;
    struct section_s*     next;
} section_t;

void init_sections();
section_t* get_current_section();
section_t* get_section_id(int id);
int  add_section(int pass, section_type_t type, int address);
int  add_opcode(int pass, int iopcode, int val);
int  add_data(int pass, char c);

#endif

/**
 * \} Sections
 * \} gbas
 */
