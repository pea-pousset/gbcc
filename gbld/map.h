/**
 * \addtogroup gbld
 * \{
 * \addtogroup Map
 * \{
 */

#include "../common/objfile.h"

#define  ALLOC_FAILED   0x10000

void     init_map();
void     free_map();
unsigned allocate(const char* filename, section_entry_t* sect);

/**
 * \} Map
 * \} gbld
 */
