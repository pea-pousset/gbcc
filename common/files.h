#ifndef FILES_H
#define FILES_H

/**< Describes a source file type */
typedef enum
{
    C,      /**< c source file, .c extension required */
    I,      /**< preprocessed c source file, .i extension required */
    S,      /**< assembly source file, .s extension required */
    O       /**< object file (default) */
} filetype_t;

typedef struct
{
    char*      name;
    filetype_t type;
    int        tmp;   /**< nonzero means the file is not a final product and
                           must be removed after treatment */
} sourcefile_t;


void            file_add(const char* name);
void            file_first();
void            file_set_attr(filetype_t type, int tmp);
sourcefile_t*   file_next();
const char*     file_name();
int             file_count();

#endif
