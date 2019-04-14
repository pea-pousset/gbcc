/**
 * \addtogroup gbld
 * \{
 * \addtogroup lists
 * \{
 */

#include "../common/objfile.h"

#define SECT_TREATED    1   /**< Section treated flag */
#define RELOC_TREATED   1   /**< Relocation treated flag */

typedef struct list_s
{
    int            file_id; /**< id of the file in which the element has been
                             * created
                             */
    void*          data;
    int            flags;   /**< General purpose flags/placeholder value.
                             * In the sections and relocations lists it serves
                             * as a marker for a treated file. In the symbol
                             * lists it holds the id of the file containing
                             * the symbol.
                             */
    struct list_s* next;
} list_t;

extern list_t* lfiles;
extern list_t* lsections;
extern list_t* lsymbols;
extern list_t* lrelocations;

void    list_add(list_t** list, int file_id, void* data, int flags);
void    list_free(list_t* list);
list_t* list_at(list_t* list, unsigned index);

/**
 * \} lists
 * \} gbld
 */

