#include "files.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct file_entry_s file_entry_t;
struct file_entry_s
{
    sourcefile_t  file;
    file_entry_t* next;
};

static int count = 0;
static file_entry_t root = { {NULL, 0, 0}, NULL };
static file_entry_t* cur = &root;

/*========================================================================*//**
 * Add a file to the source list
 *//*=========================================================================*/
void file_add(const char* name)
{
    char* p;
    file_entry_t* nf = (file_entry_t*)mmalloc(sizeof(file_entry_t));
    nf->next = NULL;
    cur->next = nf;
    cur = cur->next;


    nf->file.name = (char*)mmalloc(strlen(name) + 1);
    strcpy(nf->file.name, name);
    p = nf->file.name;

    while (*p)
        ++p;
    while (*p != '.' && p != nf->file.name)
        --p;

    if (strcmp(p, ".c") == 0 || strcmp(p, ".C") == 0)
        nf->file.type = C;
    else if (strcmp(p, ".i") == 0 || strcmp(p, ".I") == 0)
        nf->file.type = I;
    else if (strcmp(p, ".s") == 0 || strcmp(p, ".S") == 0)
        nf->file.type = S;
    else
        nf->file.type = O;

    nf->file.tmp = 0;
    ++count;
}


/*========================================================================*//**
 * Set the first file of the list as the current file
 *//*=========================================================================*/
void file_first()
{
    cur = &root;
}


/*========================================================================*//**
 * Change the current file attributes and modify its extension if necessary
 *
 * \param type: the new file type
 * \param tmp: non-zero marks the file as a temporary file
 * \todo make it work with filenames with no extension
 *//*=========================================================================*/
void file_set_attr(filetype_t type, int tmp)
{
    char* p = cur->file.name;
    while (*p)
        ++p;
    while (*p != '.' && p != cur->file.name)
        --p;

    if (*p == '.')
        *p = 0;

    cur->file.name = (char*)mrealloc(cur->file.name, strlen(cur->file.name) + 3);

    p = cur->file.name;
    while (*p != 0)
        ++p;

    *p++ = '.';
    *p++ = "ciso"[type];
    *p = 0;

    cur->file.type = type;
    cur->file.tmp = tmp;
}


/*========================================================================*//**
 *
 *//*=========================================================================*/
sourcefile_t* file_next()
{
    if (cur)
        cur = cur->next;
    return &cur->file;
}


/*========================================================================*//**
 * Return the current file name
 *//*=========================================================================*/
const char* file_name()
{
    return cur->file.name;
}

int file_count()
{
    return count;
}
