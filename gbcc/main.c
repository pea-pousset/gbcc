/** \defgroup gbcc gbcc
 * C compiler
 * \addtogroup gbcc
 * \{
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../common/errors.h"
#include "../common/options.h"
#include "../common/utils.h"
#include "../common/files.h"
#include "version.h"
#include "pp.h"
#include "parser.h"
#include "ast.h"
#include "syms.h"

const char* const pgm = "gbcc";

void help();
void version();
void on_fatal_error(int from_program);

int main(int argc, char** argv)
{
    sourcefile_t* file = NULL;
    char* output_name = NULL;

    int donot_compile = 0;
    int donot_assemble = 0;
    int donot_link = 0;

    esetprogram(pgm);
    esetonfatal(&on_fatal_error);

    parse_options(argc, argv, GBCC, &help, &version);

    if (errors())
        return EXIT_FAILURE;

    donot_compile = get_option("-E")->set;
    donot_assemble = get_option("-S")->set;
    donot_link = get_option("-c")->set;

    output_name = (char*)mmalloc(strlen(get_option("-o")->value.str) + 1);
    strcpy(output_name, get_option("-o")->value.str);

    /* .c source files */
    file_first();
    while ((file = file_next()) != NULL)
    {
        if (file->type == C)
        {
            FILE* infile = NULL;
            FILE* outfile = NULL;
            char* iname = NULL;
            char* oname = m_tmpnam();
            int   del = file->tmp;

            iname = (char*)mmalloc(strlen(file->name) + 1);
            strcpy(iname, file->name);

            esetfile(file->name);

            /*printf("*** preprocessor in: %s\n", iname);
              printf("*** tmp out: %s\n", oname); */

            if (! (infile = fopen(iname, "rb")) )
            {
                ccerr(F, "unable to open \"%s\"", file->name);
                continue;
            }

            if (! (outfile = fopen(oname, "wb")) )
            {
                fclose(infile);
                ccerr(F, "unable to create output file");
                continue;
            }

            if (pp(iname, infile, outfile))
            {
                /* Update file attributes in the list to get the output
                file name. In case of error the type is not changed and will
                be ignored by further compilation steps. */
                file_set_attr(I, !donot_compile);

                /* Copy the tmp file content to the output file */
                fclose(outfile);
                if (donot_compile && output_name)
                {
                    if (!copy_file(oname, output_name))
                        err(F, "could not write to the output file");
                    /* printf("*** preprocessor out: %s\n\n", file_name()); */
                }
                else
                {
                    if (!copy_file(oname, file_name()))
                        err(F, "could not write to the output file");
                }
            }
            else
                fclose(outfile);

            fclose(infile);
            remove(oname);
            if (del)
                remove(iname);

        }
    }
/*
    file_top();
    printf("\nRemaining files: \n");
    while((file = file_next()) != NULL)
        printf("* %s %s\n", file->name, file->tmp ? "to be removed" : "end product");
    putchar('\n');
*/

    file_first();
    while ((file = file_next()) != NULL && !donot_compile)
    {
        if (file->type == I)
        {
            FILE* infile = NULL;
            FILE* outfile = NULL;
            char* iname = NULL;
            char* oname = m_tmpnam();
            int   del = file->tmp;

            iname = (char*)mmalloc(strlen(file->name) + 1);
            strcpy(iname, file->name);

            esetfile(file->name);

            /* printf("*** parser in: %s\n", iname);
               printf("*** tmp out: %s\n", oname); */

            if (! (infile = fopen(iname, "rb")) )
            {
                ccerr(E, "unable to open \"%s\"", file->name);
                continue;
            }

            if (! (outfile = fopen(oname, "wb")) )
            {
                fclose(infile);
                ccerr(E, "unable to create output file");
                continue;
            }

            if (parse(infile, outfile))
            {
                file_set_attr(S, !donot_assemble);
                fclose(outfile);
                if (donot_assemble && output_name)
                {
                    if (!copy_file(oname, output_name))
                        err(F, "could not write to the output file");
                }
                else
                {
                    if (!copy_file(oname, file_name()))
                        err(F, "could not write to the output file");
                }
            }
            else
                fclose(outfile);

            fclose(infile);
            remove(oname);
            if (del)
                remove(iname);

        }
    }
/*
    file_top();
    printf("\nRemaining files: \n");
    while((file = file_next()) != NULL)
        printf("* %s %s\n", file->name, file->tmp ? "to be removed" : "end product");
*/

    return EXIT_SUCCESS;
}

void help()
{
    puts("Usage: gbcc [options] file...");
    puts("Options:");
    puts("  --help           Display this information.");
    puts("  --version        Display compiler version information.");
    puts("  -ftabstop=width  ");
    puts("  -E               Preprocess only; do not compile, assemble or link.");
    puts("  -S               Compile only; do not assemble or link.");
    puts("  -c               Compile and assemble, but do not link.");
    puts("  -o <file>        Place the output into <file>.");
}

void version()
{
    printf("%s %d.%d\n", pgm, VERSION_MAJOR, VERSION_MINOR);
}

void on_fatal_error(int from_program)
{
    ast_free();
    syms_free();

    if (from_program)
    {
        fprintf(stderr, "compilation terminated.\n");
        exit(EXIT_FAILURE);
    }
}

/* \} */
