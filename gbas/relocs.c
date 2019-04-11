/**
 * \addtogroup gbas
 * \{
 * \defgroup Relocations
 * \addtogroup Relocations
 * \{
 */

#include "relocs.h"

#include <stdlib.h>

#include "../common/utils.h"
#include "../common/objfile.h"

typedef struct reloc_s
{
    int sym_id;
    int section_id;
    int offset;
    int flags;
    struct reloc_s* next;
} reloc_t;

static reloc_t* root = NULL;
static reloc_t* cur = NULL;
static int relocs_count;

void init_relocs()
{
    free_relocs();
    relocs_count = 0;
}

void free_relocs()
{
    reloc_t* next = root;
    while (next)
    {
        cur = next;
        next = cur->next;
        free(cur);
    }
    root = NULL;
    cur = NULL;
}

void add_reloc(int sym_id, int section_id, int offset, int relative)
{
    reloc_t* new = (reloc_t*)mmalloc(sizeof(reloc_t));
    new->sym_id = sym_id;
    new->section_id = section_id;
    new->offset = offset;
    if (relative)
        new->flags = 1;
    else
        new->flags = 0;
    new->next = NULL;
    
    if (root == NULL)
    {
        root = new;
        cur = root;
    }
    else
    {
        cur->next = new;
        cur = new;
    }
        
    ++relocs_count;
}

void write_relocs()
{
    block_header_t header;
    reloc_entry_t reloc;
    if (relocs_count == 0)
        return;
    
    header.type = relocations;
    header.num_entries = relocs_count;
    write_block_header(&header);
    
    cur = root;
    while (cur)
    {
        reloc.sym_id = cur->sym_id;
        reloc.section_id = cur->section_id;
        reloc.offset = cur->offset;
        reloc.flags = cur->flags;
        write_reloc_entry(&reloc);
        cur = cur->next;
    }
}

/**
 * \} Relocations
 * \} gbas
 */
