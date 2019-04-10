#ifndef GBMMAP_H
#define GBMMAP_H

typedef enum {
    rom_0,
    rom_n,
    vram,
    ram,
    wram_0,
    wram_n,
    echo,
    oam,
    unusable,
    io,
    hram,
    ie,
    out_of_mmap
} gbspace_t;

gbspace_t get_space(unsigned address);
unsigned  get_map_section_size(gbspace_t sect);

#endif

