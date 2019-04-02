#include "sections.h"

#include <stdio.h>
#include <stdlib.h>

#include "../common/errors.h"
#include "commons.h"
#include "opcodes.h"
#include "outfile.h"

static section_t* root;
static section_t* cur;
static int num_sections;

static void write_section_header();

void init_sections()
{
    cur = root;
    while (cur)
    {
        root = cur;
        cur = cur->next;
        free(root);
    }

    root = NULL;
    cur = NULL;
    num_sections = 0;
}

section_t* get_current_section()
{
    return cur;
}

section_t* get_section_id(int id)
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

int add_section(int pass, section_type_t type, int address)
{
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
    else
    {
        if (cur->next == NULL)
        {
            cur = root;
            write_block_header(sections, num_sections);
        }
        else
            cur = cur->next;

        cur->pc = 0;

        write_section_header();
    }

    return 1;
}

int add_opcode(int pass, int iopcode, int val)
{
    if (root == NULL)
        err(F, "code generation before a section has been created");

    if (opcodes[iopcode].pre)
    {
        if (!add_data(pass, opcodes[iopcode].pre))
            return 0;
    }

    if (!add_data(pass, opcodes[iopcode].oc))
        return 0;

    if (!opcodes[iopcode].pre && opcodes[iopcode].len > 1)
    {
        if (opcodes[iopcode].len == 2)
        {
            if (!add_data(pass, val & 0xFF))
                return 0;
        }
        else
        {
            if (!add_data(pass, val & 0xFF))
                return 0;
            if (!add_data(pass, (val >> 8) & 0xFF))
                return 0;
        }
    }

    return 1;
}

int add_data(int pass, char c)
{
    if (root == NULL)
        err(F, "code generation before a section has been created");

    if (pass == READ_PASS)
    {
        cur->pc++;
        cur->datasize++;
        return 1;
    }

    write_int8(c);
    cur->pc++;

    return 1;
}



void write_section_header()
{
    write_int32(cur->id);
    write_int16(cur->type);
    write_int16(cur->address);
    write_int32(cur->datasize);
}

