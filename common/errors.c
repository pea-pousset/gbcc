#include "errors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "utils.h"

#ifdef _WIN32
const char* const notestr = "note";
const char* const warnstr = "warning";
const char* const errstr  = "error";
const char* const ferrstr = "fatal error";
#else
const char* const notestr = "\x1B[36mnote\x1B[0m: ";
const char* const warnstr = "\x1B[33mwarning\x1B[0m: ";
const char* const errstr  = "\x1B[31merror\x1B[0m: ";
const char* const ferrstr = "\x1B[35mfatal error\x1B[0m: ";
#endif

int eline;
int ecolumn;
static const char* program;
static void (*onfatal)() = NULL;
static char* efile = NULL;
static int error_count = 0;
static int warning_count = 0;
static int is_fatal = 0;
static int last_line = -1;
static int last_col = -1;
static int last_lvl = W;

static void verr(errtype_t type, const char* message, va_list args, int prgm);

/**=======================================================================*//**
 * Set the program name to display in further error messages
 *//*=========================================================================*/
void esetprogram(const char* const name)
{
    program = name;
}

/**=======================================================================*//**
 * Set the function to call in case of fatal error
 *//*=========================================================================*/
void esetonfatal(void(*func)())
{
    onfatal = func;
}

/**=======================================================================*//**
 * Set the file name to display in further error messages
 *//*=========================================================================*/
void esetfile(const char* name)
{
    efile = (char*)mrealloc(efile, strlen(name) + 1);
    strcpy(efile, name);
    last_line = -1;
    last_col = -1;
    last_lvl = W;
}

/**=======================================================================*//**
 * Return the number of errors
 *//*=========================================================================*/
int errors()
{
    return error_count;
}

/**=======================================================================*//**
 * Return the number of warnings
 *//*=========================================================================*/
int warnings()
{
    return warning_count;
}

/**=======================================================================*//**
 * Return true if the last error is fatal
 *//*=========================================================================*/
int fatal()
{
    return is_fatal;
}

/**=======================================================================*//**
 * Reset the number of errors
 *//*=========================================================================*/
void clear_errors()
{
    error_count = 0;
    warning_count = 0;
}

/**=======================================================================*//**
 * Reset the 'fatal' flag
 *//*=========================================================================*/
void clear_fatal()
{
    is_fatal = 0;
}

void err(errtype_t type, const char* message, ...)
{
    va_list args;
    /* if (eline == last_line && ecolumn == last_col && (int)type <= last_lvl)
        return; */


    fprintf(stderr, "%s:%d:%d: ", efile, eline, ecolumn);
    va_start(args, message);
    verr(type, message, args, 0);
    va_end(args);
}

void ccerr(errtype_t type, const char* message, ...)
{
    va_list args;
    /* if (eline == last_line && ecolumn == last_col && (int)type <= last_lvl)
        return; */


    fprintf(stderr, "%s: ", program);
    va_start(args, message);
    verr(type, message, args, 1);
    va_end(args);
}

void verr(errtype_t type, const char* message, va_list args, int prgm)
{
    const char* s;

    last_line = eline;
    last_col = ecolumn;
    last_lvl = type;

    switch (type)
    {
        case F: s = ferrstr; ++error_count; is_fatal = 1; break;
        case E: s = errstr; ++error_count; break;
        case W: s = warnstr; ++warning_count; break;
        case N: s = notestr; break;
    }

    fprintf(stderr, "%s", s);
    vfprintf(stderr, message, args);
    fputc('\n', stderr);

    if (type == F && onfatal)
        (*onfatal)(prgm);
}
