#include "map.h"

#include "../common/utils.h"
#include "../common/errors.h"
#include "rom.h"

typedef struct sectlist_s
{
    section_t*          sect;
    struct sectlist_s*  prev;
    struct sectlist_s*  next;
} sectlist_t;

sectlist_t* linked = NULL;
sectlist_t* unlinked = NULL;

static int  overlap(section_t* t1, section_t* t2);
static void add_to_list(section_t* sect, sectlist_t** list);
static void remove_from_list(section_t* sect, sectlist_t** list);

void free_sections()
{
    sectlist_t* p;
    p = linked;
    while (p)
    {
        linked = p;
        p = p->next;
        if (!linked->sect->header)
        {
            free(linked->sect->data);
            free(linked->sect);
        }
        free(linked);
    }
}

void add_section(section_t* sect)
{
    if (sect->type == org)
    {
        int i, c = 0;
        char* bank = get_from_org(sect->address);
        sectlist_t* plist = linked;

        while (plist)
        {
            if (overlap(sect, plist->sect))
                err(F, "section overlapping");  /* BETTER ERROR MESSAGE REQUIRED ####### */
            plist = plist->next;
        }

        /* CHECK FOR DATA SIZE < BANK LIMIT ########################################## */

        memcpy(bank + sect->address, sect->data, sect->datasize);

        printf("*** adding org section ***\n");
        fprintf(stderr, "id: %d\n", sect->id);
        fprintf(stderr, "type: %s\n", sect->type == org ? ".org" : "???");
        fprintf(stderr, "address: %04X\n", sect->address);
        fprintf(stderr, "size: %d bytes\n", sect->datasize);
        for (i = 0; i < sect->datasize; ++i)
        {
            ++c;
            fprintf(stderr, "%02X ", (unsigned)sect->data[i]);
            if (c == 8)
                fprintf(stderr, "   ");
            if (c == 16)
             {fprintf(stderr, "\n"); c = 0; }
        }
        fprintf(stderr, "\n\n");

        add_to_list(sect, &linked);
    }
}

void add_to_list(section_t* sect, sectlist_t** list)
{
    if (*list == NULL)
    {
        *list = (sectlist_t*)mmalloc(sizeof(sectlist_t));
        (*list)->sect = sect;
        (*list)->prev = NULL;
        (*list)->next = NULL;
    }
    else
    {
        sectlist_t* plist = *list;
        while (plist->next)
            plist = plist->next;

        plist->next = (sectlist_t*)mmalloc(sizeof(sectlist_t));
        plist->next->sect = sect;
        plist->next->prev = plist;
        plist->next->next = NULL;
    }
}

static void remove_from_list(section_t* sect, sectlist_t** list)
{
    sectlist_t* plist = *list;
    while (plist->sect != sect)
        plist = plist->next;

    if (plist->prev)
        plist->prev->next = plist->next;
    if (plist->next)
        plist->next->prev = plist->prev;
}




int overlap(section_t* t1, section_t* t2)
{
    if (t2->address >= t1->address && t2->address < t1->address + t1->datasize)
        return 1;

    if (t1->address >= t2->address && t1->address < t2->address + t2->datasize)
        return 1;

    return 0;
}
