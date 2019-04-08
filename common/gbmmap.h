#ifndef GBMMAP_H
#define GBMMAP_H

typedef enum {
    rom_0,
    rom_n,
    vram,
    extram_n,
    ram_0,
    ram_n,
    echo,
    oam,
    unusable,
    io,
    hram,
    ie,
    out_of_mmap
} gbspace_t;

gbspace_t get_space(unsigned address);

#endif

