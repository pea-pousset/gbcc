#include "rom.h"

#include <stdlib.h>
#include <string.h>
#include "../common/utils.h"

#define MAX_ROM_BANKS   512

static char* rombanks[MAX_ROM_BANKS];
static char dummy[BANK_SIZE];

void init_rom()
{
    memset(rombanks, 0, MAX_ROM_BANKS);
    rombanks[0] = (char*)mmalloc(BANK_SIZE);
    rombanks[1] = (char*)mmalloc(BANK_SIZE);
    memset(rombanks[0], 0, BANK_SIZE);
    memset(rombanks[1], 0, BANK_SIZE);
}

void free_rom()
{
    int i;
    for (i = 0; i < MAX_ROM_BANKS; ++i)
        free(rombanks[i]);
}

char* get_from_org(int org)
{
    if (org > 0x8000)
        return dummy;
    return rombanks[org / BANK_SIZE];
}

char* get_rom_bank(int bank)
{
    if (bank < 512)
    {
        if (!rombanks[bank])
        {
            rombanks[bank] = (char*)mmalloc(BANK_SIZE);
            memset(rombanks[bank], 0, BANK_SIZE);
        }
        return rombanks[bank];
    }
    return NULL;
}
