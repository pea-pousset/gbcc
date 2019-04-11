/**
 * \addtogroup gbas
 * \{
 * \defgroup Sections
 * \addtogroup Sections
 * \{
 */

#include "sections.h"

#include <stdio.h>
#include <stdlib.h>

#include "../common/errors.h"
#include "../common/objfile.h"
#include "commons.h"
#include "opcodes.h"

static section_t* root = NULL;  /**< Root of the sections list */
static section_t* cur;          /**< Current section */
static int num_sections;

/*========================================================================*//**
 * Init the sections list
 *//*=========================================================================*/
void init_sections()
{
    free_sections();
    num_sections = 0;
}

/*========================================================================*//**
 * Free the sections list
 *//*=========================================================================*/
void free_sections()
{
    section_t* next = root;
    while (next)
    {
        cur = next;
        next = cur->next;
        free(cur);
    }
    root = NULL;
    cur = NULL;
}

/*========================================================================*//**
 * Return the current section
 *//*=========================================================================*/
section_t* get_current_section()
{
    return cur;
}

/*========================================================================*//**
 * Find a section by id
 *
 * \param id: id of the section to find
 * \return a pointer to the section if it has been found, NULL otherwise
 *//*=========================================================================*/
section_t* get_section_by_id(int id)
{
    section_t* ps = root;
    while (ps)
    {
        if (ps->id == id)
            break;
        ps = ps->next;
    }
    return ps;
}

/*========================================================================*//**
 * Create a new section and set it as the current section
 *
 * \param pass: assembly pass
 * \param type: the new section's type
 * \param address: the new section's address
 *//*=========================================================================*/
void add_section(int pass, section_type_t type, int address)
{
    section_entry_t sect_entry;
    
    /* PASS 1 : Create a new section */
    if (pass == READ_PASS)
    {
        section_t* new = (section_t*)malloc(sizeof(section_t));
        if (new == NULL)
            ccerr(F, "unable to allocate memory");

        if (!root)
        {
            root = new;
            cur = root;
        }
        else
        {
            cur->next = new;
            cur = cur->next;
        }

        cur->type = type;
        cur->id = num_sections;
        cur->address = address;
        cur->pc = 0;
        cur->datasize = 0;
        cur->next = NULL;
        ++num_sections;
    }
    else /* PASS 2 */
    {
        if (cur->next == NULL) /* First section : write the block header */
        {
            block_header_t header;
            header.type = sections;
            header.num_entries = num_sections;
            cur = root;
            write_block_header(&header);
        }
        else
            cur = cur->next;

        cur->pc = 0;

        /* New section : use new header */
        sect_entry.id = cur->id;
        sect_entry.type = cur->type;
        sect_entry.address_or_bank = cur->address;
        sect_entry.data_size = cur->datasize;
        write_section_entry(&sect_entry);
    }
}

/*========================================================================*//**
 * Add an opcode to the current section
 *
 * \param pass: assembly pass
 * \param iopcode: index of the opcode in opcodes
 * \param val: the opcode argument
 *//*=========================================================================*/
void add_opcode(int pass, int iopcode, int val)
{
    if (root == NULL)
        err(F, "code generation before a section has been created");

    if (opcodes[iopcode].pre)
        add_data(pass, opcodes[iopcode].pre);
    add_data(pass, opcodes[iopcode].oc);

    if (!opcodes[iopcode].pre && opcodes[iopcode].len > 1)
    {
        if (opcodes[iopcode].len == 2)
            add_data(pass, val & 0xFF);
        else
        {
            add_data(pass, val & 0xFF);
            add_data(pass, (val >> 8) & 0xFF);
        }
    }
}

/*========================================================================*//**
 * Add 1 byte of data to the current section
 *
 * \param pass: assembly pass
 * \param c: byte value to add
 *//*=========================================================================*/
void add_data(int pass, char c)
{
    if (root == NULL)
        err(F, "code generation before a section has been created");

    if (pass == READ_PASS)
    {
        cur->pc++;
        cur->datasize++;
    }
    else
    {
        write_byte(c);
        cur->pc++;
    }
}

/**
 * \} Sections
 * \} gbas
 */
