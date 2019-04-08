#include "gbmmap.h"

gbspace_t get_space(unsigned address)
{
    if (address < 0x4000)
        return rom_0;
    if (address < 0x8000)
        return rom_n;
    if (address < 0xA000)
        return extram_n;
    if (address < 0xC000)
        return ram_0;
    if (address < 0xE000)
        return ram_n;
    if (address < 0xFE00)
        return oam;
    if (address < 0xFEA0)
        return unusable;
    if (address < 0xFF80)
        return io;
    if (address < 0xFFFF)
        return hram;
    if (address == 0xFFFF)
        return ie;
    
    return out_of_mmap;
}
