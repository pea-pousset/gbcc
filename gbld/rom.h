/**
 * \addtogroup gbld
 * \{
 * \addtogroup ROM
 * \{
 */

#ifndef ROM_H
#define ROM_H

void           init_rom();
void           free_rom();
unsigned char* get_rom_from_org(int org);
unsigned char* get_rom_bank(int bank);
void           fix_rom();

#endif

/**
 * \} ROM
 * \} gbld
 */
