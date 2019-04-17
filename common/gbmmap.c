#include "gbmmap.h"

#include "../common/errors.h"

static int num_rom_banks = 2;
static int num_ram_banks = 1;
static int num_wram_banks = 2;
static int num_vram_banks = 1;

static cartridge_t cartridge =
{
    dmg,
    rom_only,
    _32K,
    no_ram,
    {'O', 'U', 'T', '.', 'G', 'B', ' ', ' ', ' ', ' ', ' '}
};

void set_cartridge(cartridge_t cart)
{
    int romsize;
    
    if (cart.gb_type == dmg)
        num_wram_banks = 2;
    
    if (cart.cart_type == rom_only)
    {
        if (cart.rom_size != _32K)
            err(F, "ROM size too big for this type of cartridge");
        if (cart.ram_size != no_ram)
            err(F, "RAM size too big for this type of cartridge");
    }
    else
        err(F, "unknwon cartridge type");
    
    romsize = 0x8000 << (int)cart.rom_size;
    num_rom_banks = romsize / ROM_BANK_SIZE;
    
    switch(cart.ram_size)
    {
        /* In order to make .org work we need a RAM slot even if there
         is no ram */
        case no_ram:
        default: num_ram_banks = 1;
    }
}

cartridge_t get_cartridge()
{
    return cartridge;
}

int get_num_rom_banks()
{
    return num_rom_banks;
}

int get_num_ram_banks()
{
    return num_ram_banks;
}

int get_num_wram_banks()
{
    return num_wram_banks;
}

int get_num_vram_banks()
{
    return num_vram_banks;
}

gbspace_t get_space(unsigned address)
{
    if (address < 0x4000)
        return rom_0;
    if (address < 0x8000)
        return rom_n;
    if (address < 0xA000)
        return vram;
    if (address < 0xC000)
        return ram;
    if (address < 0xD000)
        return wram_0;
    if (address < 0xE000)
        return wram_n;
    if (address < 0xFE00)
        return echo;
    if (address < 0xFEA0)
        return oam;
    if (address < 0xFF00)
        return unusable;
    if (address < 0xFF80)
        return io;
    if (address < 0xFFFF)
        return hram;
    if (address == 0xFFFF)
        return ie;
    
    return out_of_mmap;
}

unsigned mmap_get_section_size(gbspace_t sect)
{
    switch (sect)
    {
        case rom_0:    return ROM_BANK_SIZE;
        case rom_n:    return ROM_BANK_SIZE;
        case vram:     return VRAM_BANK_SIZE;
        case ram:      return RAM_BANK_SIZE;
        case wram_0:   return WRAM_BANK_SIZE;
        case wram_n:   return WRAM_BANK_SIZE;
        case echo:     return 0x1E00;
        case oam:      return 0x00A0;
        case unusable: return 0x0060;
        case io:       return 0x0080;
        case hram:     return 0x007F;
        case ie:       return 0x0001;
        default:       return 0;
    }
}

unsigned mmap_addressof(gbspace_t sect)
{
    switch (sect)
    {
        case rom_0:    return 0x0000;
        case rom_n:    return 0x4000;
        case vram:     return 0x8000;
        case ram:      return 0xA000;
        case wram_0:   return 0xC000;
        case wram_n:   return 0xD000;
        case echo:     return 0xE000;
        case oam:      return 0xFE00;
        case unusable: return 0xFEA0;
        case io:       return 0xFF00;
        case hram:     return 0xFF80;
        case ie:       return 0xFFFF;
        default:       return 0;
    }
}
