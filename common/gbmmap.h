#ifndef GBMMAP_H
#define GBMMAP_H

#define MAX_ROM_BANKS   512
#define MAX_VRAM_BANKS  2
#define MAX_RAM_BANKS   16
#define MAX_WRAM_BANKS  8

#define ROM_BANK_SIZE   0x4000
#define VRAM_BANK_SIZE  0x2000
#define RAM_BANK_SIZE   0x2000
#define WRAM_BANK_SIZE  0x1000

typedef enum
{
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

typedef enum
{
    dmg = 0x00,
    /* cgb = 0x80,
    cgb_only = 0xC0 */
} gb_type_t;

typedef enum
{
    rom_only = 0x00
} cartridge_type_t;

typedef enum
{
    _32K = 0x00
} rom_size_t;

typedef enum
{
    no_ram = 0x00
} ram_size_t;

typedef struct cartridge_s
{
    gb_type_t        gb_type;
    cartridge_type_t cart_type;
    rom_size_t       rom_size;
    ram_size_t       ram_size;
    char             title[11];
} cartridge_t;


void        set_cartridge(cartridge_t type);
cartridge_t get_cartridge();
int         get_num_rom_banks();
int         get_num_ram_banks();
int         get_num_wram_banks();
int         get_num_vram_banks();
gbspace_t   get_space(unsigned address);
unsigned    mmap_get_section_size(gbspace_t sect);
unsigned    mmap_addressof(gbspace_t sect);

#endif

