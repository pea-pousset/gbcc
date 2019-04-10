#include "gbmmap.h"

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

unsigned get_map_section_size(gbspace_t sect)
{
    switch (sect)
    {
        case rom_0:    return 0x4000;
        case rom_n:    return 0x4000;
        case vram:     return 0x2000;
        case ram:      return 0x2000;
        case wram_0:   return 0x1000;
        case wram_n:   return 0x1000;
        case echo:     return 0x1E00;
        case oam:      return 0x00A0;
        case unusable: return 0x0060;
        case io:       return 0x0080;
        case hram:     return 0x007F;
        case ie:       return 0x0001;
        default:       return 0;
    }
}
