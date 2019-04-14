/**
 * \addtogroup Commons
 * \{
 * \defgroup Options
 * \addtogroup Options
 * \}
 */

#include "options.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32
    #include <windows.h>
#elif linux
    #include <unistd.h>
    #include <linux/limits.h>
#else
    #include <unistd.h>
    #include <limits.h>
#endif

#ifndef PATH_MAX
    #ifdef MAX_PATH
        #define PATH_MAX    MAX_PATH
    #else
        #define PATH_MAX    4096
    #endif
#endif

#include "../common/gbmmap.h"
#include "errors.h"
#include "utils.h"
#include "files.h"

option_t options[NUM_OPTIONS] =
{
    { "--help",      flag,   {.num = 0 },   NULL,       0, 1, 1 },
    { "--version",   flag,   {.num = 0 },   NULL,       0, 1, 1 },
    { "-ftabstop=",  number, {.num = 8 },   NULL,       0, 1, 0 },
    { "-E",          flag,   {.num = 0 },   NULL,       0, 0, 0 },
    { "-S",          flag,   {.num = 0 },   NULL,       0, 0, 0 },
    { "-c",          flag,   {.num = 0 },   NULL,       0, 1, 0 },
    { "-o",          string, {.str = NULL}, "filename", 0, 1, 1 }
};

static cartridge_t cartridge =
{
    dmg,
    rom_only,
    _32K,
    no_ram,
    {'O', 'U', 'T', '.', 'G', 'B', ' ', ' ', ' ', ' ', ' '}
};

static void read_number(char* p, option_t* opt);
static void read_string(option_t* opt, int* iarg, int argc, char** argv);

/*========================================================================*//**
 * \param program: One of GBCC, GBAS or GBLD
 * \param help: Pointer to a function displaying the help message
 * \param version: Pointer to a function displaying the version message
 *//*=========================================================================*/
void parse_options(int argc, char** argv, int program, void(*help)(),
                   void(*version)())
{
    int i;
    option_t* opt;

    opt = get_option("-o");
    opt->value.str = (char*)mmalloc(7);
    strcpy(opt->value.str, "out.gb");

    for (i = 1; i < argc; ++i)
    {
        char* p = argv[i];
        if (*p == '-')
        {
            opt = get_option(p);
            if (!opt)
                continue;
            if (strcmp(opt->name, "--help") == 0)
                opt->set = 1;
            else if (strcmp(opt->name, "--version") == 0)
                opt->set = 1;
        }
    }

    /* Leave the function before source files are added */
    if (get_option("--help")->set)
    {
        (*help)();
        return;
    }

    if (get_option("--version")->set)
    {
        (*version)();
        return;
    }

    for (i = 1; i < argc; ++i)
    {
        char* p = argv[i];
        if (*p == '-')
        {
            /* HANDLE ADDITIONAL INCLUDE PATHS LIB PATHS ETC HERE */
            if ((opt = get_option(argv[i])))
            {
                p += strlen(opt->name);

                if (program == GBAS && !opt->as_opt)
                {
                    ccerr(E, "unrecognized command line option '%s'.", argv[i]);
                    continue;
                }

                if (program == GBLD && !opt->ld_opt)
                {
                    ccerr(E, "unrecognized command line option '%s'.", argv[i]);
                    continue;
                }

                opt->set = 1;
                if (opt->type != flag)
                {
                    if (opt->type == number)
                        read_number(p, opt);
                    else if (opt->type == string)
                        read_string(opt, &i, argc, argv);
                }
                else if (*p)
                {
                    /* Option name matched but there are additional
                    characters */
                    ccerr(E, "unrecognized command line option '%s'.", argv[i]);
                    continue;
                }
            }
            else
                ccerr(E, "unrecognized command line option '%s'.", argv[i]);
        }
        else
        {
            file_add(argv[i]);
        }
    }

    if (!file_count())
        ccerr(F, "no input files.");

    if ( (get_option("-E")->set | get_option("-S")->set | get_option("-c")->set)
         && get_option("-o")->set)
    {
        if (file_count() > 1)
        {
            if (program == GBCC)
                ccerr(F, "cannot specify -o with -E, -S or -c with multiple "
                         "files");
            else
                ccerr(F, "cannot specify -o with -c with multiple files");

        }
    }

    opt = get_option("-ftabstop=");
    if (opt->value.num < 1 || opt->value.num > 100)
    {
        opt->value.num = 8;
        opt->set = 0;
    }
    
    set_cartridge(cartridge);
}




/*========================================================================*//**
 *
 *//*=========================================================================*/
option_t* get_option(const char* name)
{
    int i;
    for (i = 0; i < NUM_OPTIONS; ++i)
    {
         if (strncmp(name, options[i].name, strlen(options[i].name)) == 0)
            return options + i;
    }
    return NULL;
}




char* gen_options_str(int program)
{
    int i;
    int len = 0;
    char buf[4096];
    char* str = (char*)mmalloc(1);
    str[0] = '\0';

    for (i = 0; i < NUM_OPTIONS; ++i)
    {
        if (!options[i].set)
            continue;
        if (program == GBAS && !options[i].as_opt)
            continue;
        if (program == GBLD && !options[i].ld_opt)
            continue;

        if (options[i].type == flag)
        {
            sprintf(buf, " %s", options[i].name);
        }
        else if (options[i].type == number)
        {
            sprintf(buf, " %s%d", options[i].name, options[i].value.num);
        }
        else if (options[i].type == string)
        {
            int j;
            int delim = 0;
            for (j = 0; j < strlen(options[i].value.str); ++j)
            {
                if (isspace(options[i].value.str[j]))
                {
                    delim = 1;
                    break;
                }
            }

            if (delim)
                sprintf(buf, " %s \"%s\"", options[i].name, options[i].value.str);
            else
                sprintf(buf, " %s %s", options[i].name, options[i].value.str);
        }

        str = (char*)mrealloc(str, len + strlen(buf) + 1);
        strcpy(str + len, buf);
        len += strlen(buf);
    }

    return str;
}




/*========================================================================*//**
 *
 *//*=========================================================================*/
void read_number(char* p, option_t* opt)
{
    int val = 0;

    if (!*p)
    {
        ccerr(E, "missing argument to '%s'", opt->name);
        return;
    }

    while (*p)
    {
        if (!isdigit(*p))
        {
            ccerr(E, "argument to '%s' should be a non-negative integer.",
                opt->name);
            return;
        }
        val *= 10;
        val += *p - '0';
        ++p;
    }

    opt->value.num = val;
}




/*========================================================================*//**
 *
 *//*=========================================================================*/
void read_string(option_t* opt, int* iarg, int argc, char** argv)
{
    char *p = argv[*iarg] + strlen(opt->name);

    /* Option names not terminated with '=' can have their paramater defined
    by the next argument. */
    if (!*p && (*(p-1) == '=' || *iarg == argc - 1))
    {
        ccerr(E, "missing %s after '%s'", opt->argname, opt->name);
        return;
    }

    if (!*p)
    {
        p = argv[*iarg + 1];
        if (!*p)
        {
            ccerr(E, "missing %s after '%s'", opt->argname, opt->name);
            return;
        }
        ++*iarg;
    }

    opt->value.str = (char*)mrealloc(opt->value.str, strlen(p) + 1);
    strcpy(opt->value.str, p);
}

/**
 * \} Options
 * \} Commons
 */
