#ifndef SECTIONS_H
#define SECTIONS_H

#include <stdlib.h>

typedef enum
{
    org
} section_type_t;

typedef struct
{
    int             file_id;
    int             id;
    section_type_t  type;
    int             address;
    int             header;
    int             reloc_address;
    size_t          datasize;
    unsigned char*  data;
} section_t;

void free_sections();
void add_section(section_t* sect);

#endif
