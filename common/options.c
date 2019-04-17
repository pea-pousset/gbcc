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

#include "../common/gbmmap.h"
#include "errors.h"
#include "utils.h"
#include "files.h"
#include "defs.h"

option_t options[NUM_OPTIONS] =
{
    { "--help",      flag,   {.num = 0 },   NULL,       0, 1, 1 },
    { "--version",   flag,   {.num = 0 },   NULL,       0, 1, 1 },
    { "-ftabstop=",  number, {.num = 8 },   NULL,       0, 1, 0 },
    { "-E",          flag,   {.num = 0 },   NULL,       0, 0, 0 },
    { "-S",          flag,   {.num = 0 },   NULL,       0, 0, 0 },
    { "-c",          flag,   {.num = 0 },   NULL,       0, 1, 0 },
    { "-o",          string, {.str = NULL}, "filename", 0, 1, 1 },
    { "-g",          flag,   {.num = 0},    NULL,       0, 1, 1 }
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
                    /* Option name matched but there were additional
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


/*========================================================================*//**
 * Generate an null-terminated array of arguments filled with non-default
 * options (set by when invoking the current program) which are compatible with
 * the program to be invoked.
 *
 * \param program: program to invoke (GBAS or GBLD)
 * \return null-terminated array of options
 *//*=========================================================================*/
char** gen_options(unsigned program)
{
    unsigned i, num_opts = 1;
    char buf[PATH_MAX];
    
    if (program == GBCC)
        return NULL;
    char** opts = (char**)mmalloc((num_opts+1) * sizeof(char*));
    opts[0] = (char*)mmalloc(5);
    opts[1] = NULL;
    
    if (program == GBAS)
        strcpy(opts[0], "gbas");
    else
        strcpy(opts[0], "gbld");
    
    for (i = 0; i < NUM_OPTIONS; ++i)
    {
        if (!options[i].set)
            continue;
        if (program == GBAS && !options[i].as_opt)
            continue;
        if (program == GBLD && !options[i].ld_opt)
            continue;
        
        opts = (char**)mrealloc(opts, (++num_opts+1) * sizeof(char*));
        opts[num_opts] = NULL;
        
        if (options[i].type == flag)    /* -opt */
        {
            opts[num_opts-1] = (char*)mmalloc(strlen(options[i].name) + 1);
            strcpy(opts[num_opts-1], options[i].name);
        }
        else if (options[i].type == number) /* -opt=<number> */
        {
            sprintf(buf, "%s%d", options[i].name, options[i].value.num);
            opts[num_opts-1] = (char*)mmalloc(strlen(buf) + 1);
            strcpy(opts[num_opts-1], buf);
        }
        else if (options[i].type == string) /* -opt <string> */
        {
            unsigned j;
            int delim = 0;
            
            opts = (char**)mrealloc(opts, (++num_opts+1) * sizeof(char*));
            opts[num_opts] = NULL;
            
            for (j = 0; j < strlen(options[i].value.str); ++j)
            {
                if (isspace(options[i].value.str[j]))
                {
                    delim = 1;
                    break;
                }
            }
            
            if (delim)
                sprintf(buf, "\"%s\"", options[i].value.str);
            else
                strcpy(buf, options[i].value.str);
            
            opts[num_opts-2] = (char*)mmalloc(strlen(options[i].name) + 1);
            opts[num_opts-1] = (char*)mmalloc(strlen(buf) + 1);
            
            strcpy(opts[num_opts-2], options[i].name);
            strcpy(opts[num_opts-1], buf);
        }
    }

    return opts;
}

char** add_option(char** options, char* opt)
{
    unsigned nopt = 0;
    while (options[nopt])
        nopt++;
    
    options = (char**)mrealloc(options, (++nopt+1) * sizeof(char*));
    options[nopt-1] = (char*)malloc(strlen(opt)+1);
    options[nopt] = NULL;
    strcpy(options[nopt-1], opt);
    
    return options;
}


void free_options(char* options[])
{
    unsigned i = 0;
    while (options[i])
        free(options[i++]);
    free(options);
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
