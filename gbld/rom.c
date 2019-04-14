/**
 * \addtogroup gbld
 * \{
 * \defgroup ROM
 * \addtogroup ROM
 * \{
 */

#include "rom.h"

#include <stdlib.h>
#include <string.h>
#include "../common/utils.h"
#include "../common/gbmmap.h"

#define header_address          0x0104
#define header_size             76
#define header_title            48
#define header_gb_type          63
#define header_cartrige_type    67
#define header_rom_size         68
#define header_ram_size         69
#define header_checksum         73
#define header_glob_checksum_hi 74
#define header_glob_checksum_lo 75

static unsigned char rom_header[76] =
{
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
    'O', 'U', 'T', '.', 'G', 'B', ' ', ' ', ' ', ' ', ' ',  /* Title */
    0xBA, 0xAD, 0xF0, 0x0D,     /* Manufacturer code */
    0,                          /* GameBoy Color compatibilty */
    0, 0,                       /* New licensee code */
    0,                          /* Super Gameboy compatibility */
    0,                          /* Cartridge Type */
    0,                          /* Rom size */
    0,                          /* Ram size */
    1,                          /* Destination code */
    0x33,                       /* Old licensee code */
    1,                          /* Rom version */
    0x00,                       /* Header checksum */
    0x00, 0x00                  /* Global checksum */
};

static unsigned char* rombanks[MAX_ROM_BANKS];
static unsigned char dummy[ROM_BANK_SIZE];
static int max_rom_bank;

void init_rom()
{
    unsigned i;
    for (i = 0; i < MAX_ROM_BANKS; ++i)
        rombanks[i] = NULL;
    rombanks[0] = (unsigned char*)mmalloc(ROM_BANK_SIZE);
    rombanks[1] = (unsigned char*)mmalloc(ROM_BANK_SIZE);
    memset(rombanks[0], 0, ROM_BANK_SIZE);
    memset(rombanks[1], 0, ROM_BANK_SIZE);
    max_rom_bank = 1;
}

void free_rom()
{
    int i;
    for (i = 0; i < max_rom_bank; ++i)
        free(rombanks[i]);
}

unsigned char* get_rom_from_org(int org)
{
    if (org > 0x8000)
        return dummy;
    return rombanks[org / ROM_BANK_SIZE];
}

unsigned char* get_rom_bank(int bank)
{
    if (bank < MAX_ROM_BANKS)
    {
        if (!rombanks[bank])
        {
            rombanks[bank] = (unsigned char*)mmalloc(ROM_BANK_SIZE);
            memset(rombanks[bank], 0, ROM_BANK_SIZE);
        }
        if (bank > max_rom_bank)
            max_rom_bank = bank;
        
        return rombanks[bank];
    }
    return NULL;
}

void fix_rom()
{
    unsigned int i, j;
    unsigned char checksum = 0;
    unsigned int global_checksum = 0;
    cartridge_t cart = get_cartridge();
    
    rom_header[header_gb_type] = cart.gb_type;
    rom_header[header_cartrige_type] = cart.cart_type;
    rom_header[header_rom_size] = cart.rom_size;
    rom_header[header_ram_size] = cart.ram_size;
    memcpy(rom_header+header_title, cart.title, 11);
    memcpy(rombanks[0]+header_address, rom_header, header_size);
    
    for (i = header_address+header_title; i<header_address+header_checksum; ++i)
        checksum -= rombanks[0][i] + 1;
    rombanks[0][header_address+header_checksum] = checksum;

    for (i = 0; i <= max_rom_bank; ++i)
    {
        if (rombanks[i] == NULL)
            continue;
        for (j = 0; j < ROM_BANK_SIZE; ++j)
        {
            if (i == 0 && (j == header_address+header_glob_checksum_hi
                           || j == header_address+header_glob_checksum_lo))
            {
                continue;
            }
            global_checksum += rombanks[i][j];
        }
        
        global_checksum &= 0xFFFF;
    }
    
    rombanks[0][header_address+header_glob_checksum_lo]
        = global_checksum & 0xFF;
    rombanks[0][header_address+header_glob_checksum_hi]
        = (global_checksum >> 8) & 0xFF;
}

/**
 * \} ROM
 * \} gbld
 */
