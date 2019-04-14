/**
 * \addtogroup gbld
 * \{
 * \defgroup lists Linked lists
 * \addtogroup lists
 * \{
 */

#include "lists.h"
#include <stdlib.h>
#include "../common/utils.h"

list_t* lfiles = NULL;
list_t* lsections = NULL;
list_t* lsymbols = NULL;
list_t* lrelocations = NULL;

void list_add(list_t** list, int file_id, void* data, int flags)
{
    list_t* elem = (list_t*)mmalloc(sizeof(list_t));
    elem->file_id = file_id;
    elem->data = data;
    elem->flags = flags;
    elem->next = NULL;
    
    if (*list == NULL)
        *list = elem;
    else
    {
        list_t* plist = *list;
        while (plist->next)
            plist = plist->next;
        plist->next = elem;
    }
}

void list_free(list_t* list)
{
    list_t* next, * cur = list;
    while (cur)
    {
        next = cur->next;
        free(cur->data);
        free(cur);
        cur = next;
    }
    list = NULL;
}

list_t* list_at(list_t* list, unsigned index)
{
    list_t* elem = list;
    if (elem == NULL)
        return NULL;
    while (index-- && elem)
        elem = elem->next;
    if (elem == NULL)
        return NULL;
    return elem;
}

/**
 * \} lists
 * \} gbld
 */
 
