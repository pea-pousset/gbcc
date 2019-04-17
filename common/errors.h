/**
 * \addtogroup Commons
 * \{
 * \addtogroup Errors
 * \{
 */
#ifndef ERRORS_H
#define ERRORS_H

#include <assert.h>
#include <string.h>

#define __FILENAME__ (strrchr(__FILE__, '\\') ? \
    strrchr(__FILE__, '\\') + 1 : __FILE__)

#ifndef NDEBUG
    #define TODO(x)  printf("[TODO: %s %s:%d]\n", x, __FILENAME__, __LINE__)
#else
    #define TODO(x)
#endif


typedef enum
{
    N,  /**< note */
    W,  /**< warning */
    E,  /**< error */
    F   /**< fatal, force the program to exit */
} errtype_t;

extern int eline;     /**< Line to display in an error message */
extern int ecolumn;   /**< Column to display in an error message */

extern const char* const notestr;
extern const char* const warnstr;
extern const char* const errstr;
extern const char* const ferrstr;

void esetprogram(const char* const name);
void esetonfatal(void (*func)(int));
void esetfile(const char* name);
int  errors();
int  warnings();
int  fatal();
void add_error();
void clear_errors();
void clear_fatal();
void err(errtype_t type, const char* message, ...);
void ccerr(errtype_t type, const char* message, ...);

#endif

/**
 * \} Errors
 * \} Commons
 */

