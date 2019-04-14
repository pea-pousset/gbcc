/**
 * \addtogroup gbld
 * \{
 * \defgroup Map
 * \addtogroup Map
 * \{
 */

#include "map.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../common/utils.h"
#include "../common/errors.h"
#include "../common/gbmmap.h"

typedef struct alloc_s
{
    int             address;
    int             size;
    char*           filename;
    struct alloc_s* prev;
    struct alloc_s* next;
} alloc_t;

typedef struct slot_s
{
    char     name[16];
    int      address;
    int      size;
    alloc_t* allocs;
} slot_t;

static slot_t*  slot_rom_0 = NULL;
static slot_t** slot_rom_n = NULL;
static slot_t** slot_vram = NULL;
static slot_t** slot_ram = NULL;
static slot_t*  slot_wram_0 = NULL;
static slot_t** slot_wram_n = NULL;
static slot_t*  slot_echo = NULL;
static slot_t*  slot_oam = NULL;
static slot_t*  slot_unusable = NULL;
static slot_t*  slot_io = NULL;
static slot_t*  slot_hram = NULL;
static slot_t*  slot_ie = NULL;

static void    free_allocs(slot_t* slot);
static void    add_alloc(const char* filename, char* sectname, slot_t* slot,
                         int address, int size);
static slot_t* get_slot_by_address(int address);

void init_map()
{
    unsigned i;
    free_map();
    
    slot_rom_0 = (slot_t*)mmalloc(sizeof(slot_t));
    slot_rom_0->address = mmap_addressof(rom_0);
    slot_rom_0->size = mmap_get_section_size(rom_0);
    slot_rom_0->allocs = NULL;
    strcpy(slot_rom_0->name, "ROM 0");
    
    slot_rom_n = (slot_t**)mmalloc(sizeof(slot_t*) * get_num_rom_banks() - 1);
    for (i = 0; i < get_num_rom_banks() - 1; ++i)
    {
        slot_rom_n[i] = (slot_t*)mmalloc(sizeof(slot_t));
        slot_rom_n[i]->address = mmap_addressof(rom_n);
        slot_rom_n[i]->size = mmap_addressof(rom_n);
        slot_rom_n[i]->allocs = NULL;
        sprintf(slot_rom_n[i]->name, "ROM %d", i + 1);
    }
    
    slot_vram = (slot_t**)mmalloc(sizeof(slot_t*) * get_num_vram_banks());
    for (i = 0; i < get_num_vram_banks(); ++i)
    {
        slot_vram[i] = (slot_t*)malloc(sizeof(slot_t));
        slot_vram[i]->address = mmap_addressof(vram);
        slot_vram[i]->size = mmap_get_section_size(vram);
        slot_vram[i]->allocs = NULL;
        sprintf(slot_vram[i]->name, "VRAM %d", i);
    }
    
    slot_ram = (slot_t**)mmalloc(sizeof(slot_t*) * get_num_ram_banks());
    for (i = 0; i < get_num_ram_banks(); ++i)
    {
        slot_ram[i] = (slot_t*)malloc(sizeof(slot_t));
        slot_ram[i]->address = mmap_addressof(ram);
        slot_ram[i]->size = mmap_get_section_size(ram);
        slot_ram[i]->allocs = NULL;
        sprintf(slot_ram[i]->name, "RAM %d", i);
    }
    
    slot_wram_0 = (slot_t*)mmalloc(sizeof(slot_t));
    slot_wram_0->address = mmap_addressof(wram_0);
    slot_wram_0->size = mmap_get_section_size(wram_0);
    slot_wram_0->allocs = NULL;
    strcpy(slot_wram_0->name, "WRAM 0");
    
    slot_wram_n = (slot_t**)mmalloc(sizeof(slot_t*) * get_num_wram_banks()-1);
    for (i = 0; i < get_num_wram_banks()-1; ++i)
    {
        slot_wram_n[i] = (slot_t*)malloc(sizeof(slot_t));
        slot_wram_n[i]->address = mmap_addressof(wram_n);
        slot_wram_n[i]->size = mmap_get_section_size(wram_n);
        slot_wram_n[i]->allocs = NULL;
        sprintf(slot_wram_n[i]->name, "WRAM %d", i+1);
    }
   
    slot_echo = (slot_t*)mmalloc(sizeof(slot_t));
    slot_echo->address = mmap_addressof(echo);
    slot_echo->size = mmap_get_section_size(echo);
    slot_echo->allocs = NULL;
    strcpy(slot_echo->name, "ECHO");
    
    slot_oam = (slot_t*)mmalloc(sizeof(slot_t));
    slot_oam->address = mmap_addressof(oam);
    slot_oam->size = mmap_get_section_size(oam);
    slot_oam->allocs = NULL;
    strcpy(slot_oam->name, "OAM");
    
    slot_unusable = (slot_t*)mmalloc(sizeof(slot_t));
    slot_unusable->address = mmap_addressof(unusable);
    slot_unusable->size = mmap_get_section_size(unusable);
    slot_unusable->allocs = NULL;
    strcpy(slot_unusable->name, "UNUSABLE");
    
    slot_io = (slot_t*)mmalloc(sizeof(slot_t));
    slot_io->address = mmap_addressof(io);
    slot_io->size = mmap_get_section_size(io);
    slot_io->allocs = NULL;
    strcpy(slot_io->name, "IO");
    
    slot_hram = (slot_t*)mmalloc(sizeof(slot_t));
    slot_hram->address = mmap_addressof(hram);
    slot_hram->size = mmap_get_section_size(hram);
    slot_hram->allocs = NULL;
    strcpy(slot_hram->name, "HRAM");
    
    slot_ie = (slot_t*)mmalloc(sizeof(slot_t));
    slot_ie->address = mmap_addressof(ie);
    slot_ie->size = mmap_get_section_size(ie);
    slot_ie->allocs = NULL;
    strcpy(slot_ie->name, "IE");
}

void free_map()
{
    unsigned i;
    
    if (!slot_rom_0)
        return;
    
    free_allocs(slot_rom_0);
    free(slot_rom_0);
    
    for (i = 0; i < get_num_rom_banks() - 1; ++i)
    {
        free_allocs(slot_rom_n[i]);
        free(slot_rom_n[i]);
        free(slot_rom_n);
    }
    
    for (i = 0; i < get_num_vram_banks(); ++i)
    {
        free_allocs(slot_vram[i]);
        free(slot_vram[i]);
        free(slot_vram);
    }
    
    for (i = 0; i < get_num_ram_banks(); ++i)
    {
        free_allocs(slot_ram[i]);
        free(slot_ram[i]);
        free(slot_ram);
    }
    
    free_allocs(slot_wram_0);
    free(slot_wram_0);
    
    for (i = 0; i < get_num_wram_banks() - 1; ++i)
    {
        free_allocs(slot_wram_n[i]);
        free(slot_wram_n[i]);
        free(slot_wram_n);
    }
    
    free_allocs(slot_echo);
    free(slot_echo);
    
    free_allocs(slot_oam);
    free(slot_oam);

    free_allocs(slot_unusable);
    free(slot_unusable);
    
    free_allocs(slot_io);
    free(slot_io);
    
    free_allocs(slot_hram);
    free(slot_hram);
    
    free_allocs(slot_ie);
    free(slot_ie);
    
    slot_rom_0 = NULL;
    slot_rom_n = NULL;
    slot_vram = NULL;
    slot_ram = NULL;
    slot_wram_0 = NULL;
    slot_wram_n = NULL;
    slot_echo = NULL;
    slot_oam = NULL;
    slot_unusable = NULL;
    slot_io = NULL;
    slot_hram = NULL;
    slot_ie = NULL;
}

/*========================================================================*//**
 * \param filename: name of the file in which the section has been created
 *//*=========================================================================*/
unsigned allocate(const char* filename, section_entry_t* sect)
{
    char buf[16];
    if (sect->type == org)
    {
        sprintf(buf, ".org $%x", sect->offset);
        add_alloc(filename, buf, get_slot_by_address(sect->offset),
                  sect->offset, sect->data_size);
        return sect->offset;
    }
    
    return ALLOC_FAILED;
}

void free_allocs(slot_t* slot)
{
    alloc_t* cur = slot->allocs;
    alloc_t* next;
    while (cur)
    {
        next = cur->next;
        free(cur->filename);
        free(cur);
        cur = next;
    }
}

void add_alloc(const char* filename, char* sectname, slot_t* slot, int address,
               int size)
{
    alloc_t* alloc = (alloc_t*)mmalloc(sizeof(alloc_t));
    alloc_t* elem;
    int inserted = 0;
    
    if (address + size > slot->address + slot->size)
    {
        ccerr(E, "%s: %s: section '%s' bounds exceeded",
              filename, sectname, slot->name);
        free(alloc);
        return;
    }
    
    alloc->address = address;
    alloc->size = size;
    alloc->filename = (char*)mmalloc(strlen(filename)+1);
    strcpy(alloc->filename, filename);
    
    if (slot->allocs == NULL)
    {
        alloc->prev = NULL;
        alloc->next = NULL;
        slot->allocs = alloc;
        inserted = 1;
    }
    
    elem = slot->allocs;
    while (!inserted && elem->next && address > elem->address)
    {
        if (elem->next == NULL && address > elem->address)
        {
            /* insert at the end of the list */
            alloc->next = NULL;
            elem->next = alloc;
            alloc->prev = elem;
            
            inserted = 1;
            break;
        }
        elem = elem->next;
    }
    
    if (!inserted)
    {
        /* Insert before the first element with a higher address */
        alloc->prev = elem->prev;
        elem->prev = alloc;
        alloc->next = elem;
        
        if (elem->prev != NULL)
            elem->prev->next = elem;
        else
            slot->allocs = elem;
    }
    
    TODO("Check for overlapping sections or full sections");
}

slot_t* get_slot_by_address(int address)
{
    if (address < mmap_addressof(rom_n))
        return slot_rom_0;
    else if (address < mmap_addressof(vram))
        return slot_rom_n[0];
    else if (address < mmap_addressof(ram))
        return slot_vram[0];
    else if (address < mmap_addressof(wram_0))
        return slot_ram[0];
    else if (address < mmap_addressof(wram_n))
        return slot_wram_0;
    else if (address < mmap_addressof(echo))
        return slot_wram_n[0];
    else if (address < mmap_addressof(oam))
        return slot_echo;
    else if (address < mmap_addressof(unusable))
        return slot_oam;
    else if (address < mmap_addressof(io))
        return slot_unusable;
    else if (address < mmap_addressof(hram))
        return slot_io;
    else if (address < mmap_addressof(ie))
        return slot_hram;
    else if (address == mmap_addressof(ie))
        return slot_ie;
    else
        return NULL;
}

/**
 * \} Map
 * \} gbld
 */
