/**
 * \addtogroup gbld
 * \{
 * \addtogroup ROM
 * \{
 */

#ifndef ROM_H
#define ROM_H

#define BANK_SIZE       0x4000

void  init_rom();
void  free_rom();
char* get_from_org(int org);
char* get_rom_bank(int bank);

#endif

/**
 * \} ROM
 * \} gbld
 */
