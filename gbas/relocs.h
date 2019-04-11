/**
 * \addtogroup gbas
 * \{
 * \addtogroup Relocations
 * \{
 */

#ifndef RELOCS_H
#define RELOCS_H

void init_relocs();
void free_relocs();
void add_reloc(int sym_id, int section_id, int offset, int relative);
void write_relocs();

#endif

/**
 * \} Relocations
 * \} gbas
 */
