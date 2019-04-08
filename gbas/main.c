/**
 * \defgroup gbas gbas
 * 2 pass assembler
 * \addtogroup gbas
 * \{
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

#include "../common/utils.h"
#include "../common/errors.h"
#include "../common/options.h"
#include "../common/files.h"
#include "commons.h"
#include "opcodes.h"
#include "sections.h"
#include "syms.h"
#include "version.h"
#include "outfile.h"

#define BUFSIZE     256

/**
 * Special token types
 */
typedef enum
{
    ID = 256,   /**< Identifier */
    KEYW,       /**< Keyword */
    NUM,        /**< Numeric constant */

    _BYTE,      /**< .byte directive */
    _WORD,      /**< .word directive */
    _ASCII,     /**< .ascii directive */
    _ORG,       /**< .org directive */

    EOL,        /**< End of line */
    ERR         /**< Invalid token */
} token_type_t;

/**
 * Structure holding a token's informations
 */
typedef struct
{
    token_type_t type;          /**< Type < 256 represents a single character */
    int          num_val;       /**< Numeric value */
    char         str[MAX_ID_LEN+1]; /**< Identifier or keyword string value */
    int          line;          /**< Line of the token in the source file */
    int          column;        /**< Column of the token in the source file */
} token_t;


const char* const pgm = "gbas";

static char  linebuf[BUFSIZE];   /**< The line of assembly being processed */
static char* lineptr;            /**< Current character in the line */
static FILE* infile = NULL;      /**< The current source file */
static FILE* outfile = NULL;     /**< The current output file */
static char* input_name = NULL;  /**< The source file name and path */
static int   line;               /**< Current line number in the source file */
static int   column;             /**< Current column in the source file */
static token_t tok;              /**< The current token */
static int   tabstop;            /**< Tabulation width in the source code */
static jmp_buf fataljmp;

void help();
void version();
void on_fatal_error(int from_program);
int  get_line();
void get_token();
void parse_line(int pass);
void parse_directive(int pass);
int  compare(const char* str1, const char* str2);
int  filter(int *lb, int *ub, int col, char c);
token_t* token();




int main(int argc, char** argv)
{
    sourcefile_t* file = NULL;
    char* output_name = NULL;
    int donot_link = 0;
    int errors_encountered = 0;

    esetprogram(pgm);
    esetonfatal(&on_fatal_error);

    parse_options(argc, argv, GBAS, &help, &version);

    if (errors())
        return EXIT_FAILURE;

    tabstop = get_option("-ftabstop=")->value.num;

    output_name = (char*)mmalloc(strlen(get_option("-o")->value.str) + 1);
    strcpy(output_name, get_option("-o")->value.str);

    file_first();
    while ((file = file_next()) != NULL)
    {
        char* oname = m_tmpnam();
        infile = NULL;
        outfile = NULL;

        input_name = (char*)mmalloc(strlen(file->name) + 1);
        strcpy(input_name, file->name);

        esetfile(file->name);

        if (! (infile = fopen(input_name, "rb")) )
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

        init_sections();
        init_syms();
        set_outfile(outfile);

        clear_errors();
        clear_fatal();
        setjmp(fataljmp);
        if (fatal())
        {
            errors_encountered = 1;
            remove(oname);
            continue;
        }

        line = 0;
        while (get_line())
            parse_line(READ_PASS);

        rewind(infile);
        write_obj_header();

        line = 0;
        while (get_line() && !errors())
            parse_line(GEN_PASS);

        write_syms();
        fclose(outfile);

        if (!errors())
        {
            file_set_attr(O, 0);    /* Update the file extension */

            if (donot_link && output_name)
            {
                if (!copy_file(oname, output_name))
                    ccerr(F, "could not write to the output file");
            }
            else
            {
                if (!copy_file(oname, file_name()))
                    ccerr(F, "could not write to the output file");
            }
        }
        else
        {
            errors_encountered = 1;
            fclose(outfile);
        }

        fclose(infile);
        remove(oname);
    }

    if (!errors_encountered && !donot_link)
    {
        char* optstr = gen_options_str(GBLD);
        char* callstr = (char*)malloc(5);
        int len = 4;
        strcpy(callstr, "gbld");

        file_first();
        while ((file = file_next()) != NULL)
        {
            callstr = (char*)mrealloc(callstr, len + strlen(file->name) + 2);
            *(callstr + len) = ' ';
            strcpy(callstr + len + 1, file->name);
            len = strlen(callstr);
        }

        callstr = (char*)mrealloc(callstr, len + strlen(optstr) + 1);
        strcpy(callstr + len, optstr);

        system(callstr);
        /* printf("%s\n", callstr); */

        free(optstr);
        free(callstr);
    }

    return errors_encountered;
}




/*========================================================================*//**
 * Display the program's help message
 *//*=========================================================================*/
void help()
{
    puts("Usage: gbas [options] file...");
    puts("Options:");
    puts("  --help          Display this information");
    puts("  --version       Display assembler version information");
    puts("  -ftabstop=width ");
    puts("  -c              Assemble only, do not link");
    puts("  -o <file>       Place the output into <file>");
}

void version()
{
    printf("%s %d.%d\n", pgm, VERSION_MAJOR, VERSION_MINOR);
}




void on_fatal_error(int from_program)
{
    if (infile)
        fclose(infile);

    if (outfile)
        fclose(outfile);

    if (from_program)
        exit(EXIT_FAILURE);
    else
        longjmp(fataljmp, 0);
}




/*========================================================================*//**
 * Read a line of assembly from the source file and place it into 'linebuf'
 *
 * \todo read char by char rather than with fgets and remove comments at once
 * \todo Handle the UTF-8 BOM for the first line
 *//*=========================================================================*/
int get_line()
{
    column = 1;

    if (!fgets(linebuf, BUFSIZE, infile))
        return 0;

    lineptr = linebuf;
    while (*lineptr && *lineptr != ';' && *lineptr != '\n')
        ++lineptr;
    *lineptr = 0;
    lineptr = linebuf;

    ++line;
    return 1;
}




/*========================================================================*//**
 * \todo Allow indentifiers starting with a '.' to implement sublabels
 *//*=========================================================================*/
void get_token()
{
    int i;
    memset(tok.str, 0, MAX_ID_LEN + 1);

    while (isspace(*lineptr))
    {
        if (*lineptr == '\t')
            column += tabstop - column % tabstop;
        ++column;
        ++lineptr;
    }

    tok.line = line;
    tok.column = column;
    eline = line;
    ecolumn = column;

    if (!*lineptr)
    {
        tok.type = EOL;
    }
    else if (*lineptr == '_' || isalpha(*lineptr))
    {
        tok.type = ID;

        i = 0;
        while (*lineptr == '_' || isalnum(*lineptr))
        {
            if (i < MAX_ID_LEN + 1)
                tok.str[i++] = *lineptr;
            ++lineptr;
            ++column;
        }
        if (i >= MAX_ID_LEN + 1)
        {
            --i;
            err(E, "identifier too long");
        }
        tok.str[i] = 0;

        for (i = 0; i < NUM_KEYWORDS; i++)
        {
            if (compare(tok.str, keywords[i]) == 0)
            {
                tok.type = KEYW;
                return;
            }
        }
    }
    else if (*lineptr == '.')
    {
        tok.type = ERR;
        tok.str[0] = '.';
        ++lineptr;
        ++column;

        i = 1;
        while (isalpha(*lineptr))
        {
            if (i < MAX_ID_LEN + 1)
                tok.str[i++] = *lineptr;
            ++lineptr;
            ++column;
        }

        if (i >= MAX_ID_LEN + 1)
        {
            --i;
            err(E, "identifier too long");
        }
        tok.str[i] = 0;

        if (compare(tok.str + 1, "BYTE") == 0)
            tok.type = _BYTE;
        else if (compare(tok.str + 1, "WORD") == 0)
            tok.type = _WORD;
        else if (compare(tok.str + 1, "ASCII") == 0)
            tok.type = _ASCII;
        else if (compare(tok.str + 1, "ORG") == 0)
            tok.type = _ORG;
    }
    else if (*lineptr == '$' || isdigit(*lineptr))
    {
        int base = 10;
        tok.type = NUM;
        tok.num_val = 0;

        if (*lineptr == '$')
        {
            base = 16;
            ++lineptr;
            ++column;
        }

        while (base == 10 ? isdigit(*lineptr) : isxdigit(*lineptr))
        {
            tok.num_val *= base;
            tok.num_val += toupper(*lineptr) - (*lineptr > '9' ? 'A'-10 : '0');

            if (tok.num_val > 0xFFFF)
                err(E, "constant too big");

            ++lineptr;
            ++column;
        }

        if (isalpha(*lineptr) || (*lineptr == '_'))
            tok.type = ERR;
    }
    else
    {
        tok.type = *lineptr++;
        ++column;
    }
}




/*========================================================================*//**
 *
 *//*=========================================================================*/
void parse_line(int pass)
{
    int ub = NUM_OPCODES - 1; /* Upper bound of the candidates opcode list */
    int lb = 0;               /* Lower bound of the candidates opcode list */
    int val = 0;    /* Numeric value, constant or placeholder for a variable */
    int neg = 0;    /* Non-zero means the numeric value is negative */
    int ccol;       /* Index of the character to test in the opcode strings */
    char exp = 0;   /* Non-zero means ] or ) is expected */
    int i;

    get_token();

    if (tok.type == EOL)
        return;

    if (tok.type >= _BYTE && tok.type <= _ORG)
    {
        parse_directive(pass);
        return;
    }

    if (tok.type == ID)
    {
        sym_declare(pass, tok.str, input_name, tok.line, tok.column);
        get_token();
        if (tok.type != ':')
        {
            err(E, "expected ':' after identifier");
            return;
        }
        get_token();
    }

    if (tok.type == EOL)
        return;
    else if (tok.type >= _BYTE && tok.type <= _ORG)
    {
        parse_directive(pass);
        return;
    }
    else if (tok.type != KEYW)
    {
        err(E, "expected instruction");
        return;
    }

    /*====== instruction ======*/

    for (i = 0; i < MAX_OP_LEN; ++i)
    {
        char c = toupper(tok.str[i]);
        if (!filter(&lb, &ub, i, c))
        {
            err(E, "expected instruction");
            return;
        }
    }

    if (lb == ub && opcodes[lb].str[ARG1_COLUMN] == ' ')
    {
        add_opcode(pass, lb, 0);
        return;
    }

    /*====== argument 1 ======*/
    get_token();
    ccol = ARG1_COLUMN;
    exp = 0;
    if (tok.type == '(' || tok.type == '[')
    {
        if (tok.type == '(')
            exp = ')';
        else
            exp = ']';

        if (!filter(&lb, &ub, ccol, '['))
        {
            err(E, "invalid argument");
            return;
        }
        get_token();
        ccol = ARG_1_BRACKETED;
    }

    if (tok.type == ID || tok.type == NUM || tok.type == '+' || tok.type == '-')
    {
        int oub = ub;
        int olb = lb;
        if (tok.type == '+' || tok.type == '-')
        {
            if (tok.type == '-')
                neg = 1;

            get_token();
            if (tok.type != NUM)
            {
                err(E, "invalid number");
                return;
            }
        }

        if (tok.type == NUM)
        {
            val = tok.num_val;
            if (neg)
                val = -val;
        }
        else if (tok.type == ID)
        {
            int relative = 0;
            if (lb >= 146 && ub <= 150) /* JRs */
                relative = 1;
            val = sym_request(pass, tok.str, relative);
        }

        if (!filter(&lb, &ub, ccol, '%'))
        {
            char nbuf[16];
            ub = oub;
            lb = olb;
            if (filter(&lb, &ub, ccol, '?'))    /* RST, BIT, SET .. */
            {
                sprintf(nbuf, "%02X", tok.num_val);
                for (i = 0; i < strlen(nbuf) + 1; i++)
                {
                    if (!filter(&lb, &ub, ccol + i + 1, nbuf[i]))
                    {
                        err(E, "invalid argument");
                        return;
                    }
                }
            }
            else
            {
                err(E, "invalid argument");
                return;
            }
        }
        else
        {
            if (opcodes[lb].str[ccol+1] == 'b')
            {
                if (val < -127 || val > 255)
                {
                    err(E, "constant too big");
                    return;
                }
            }
            else if (val < -32767 || val > 65535)
            {
                err(E, "constant too big");
                return;
            }
        }
    }
    else if (tok.type == KEYW || tok.type == EOL)
    {
        for (i = 0; i < strlen(tok.str) + 1; ++i)
        {
            char c = toupper(tok.str[i]);
            if (!filter(&lb, &ub, ccol + i, c))
            {
                err(E, "invalid argument");
                return;
            }
        }
    }
    else
    {
        err(E, "invalid argument");
        return;
    }

    if (exp)
    {
        get_token();
        if (tok.type != exp)
        {
            if (exp == ')')
                err(E, "expected ')'");
            else
                err(E, "expected ']'");
            return;
        }
        exp = 0;
    }

    if (lb == ub && opcodes[lb].str[ARG2_COLUMN] == ' ')
    {
        get_token();
        if (tok.type == EOL)
            add_opcode(pass, lb, val);
        else
            err(E, "unexpected argument");
        return;
    }

    /*===== argument 2 =====*/
    get_token();
    if (tok.type != ',')
    {
        err(E, "expected ','");
        return;
    }

    get_token();
    ccol = ARG2_COLUMN;
    exp = 0;
    if (tok.type == '(' || tok.type == '[')
    {
        if (tok.type == '(')
            exp = ')';
        else
            exp = ']';

        if (!filter(&lb, &ub, ccol, '['))
        {
            err(E, "invalid argument");
            return;
        }
        get_token();
        ccol = ARG_2_BRACKETED;
    }

    if (tok.type == ID || tok.type == NUM || tok.type == '+' || tok.type == '-')
    {
        int oub = ub;
        int olb = lb;
        if (tok.type == '+' || tok.type == '-')
        {
            if (tok.type == '-')
                neg = 1;

            get_token();
            if (tok.type != NUM)
            {
                err(E, "invalid number");
                return;
            }
        }

        if (tok.type == NUM)
        {
            val = tok.num_val;
            if (neg)
                val = -val;
        }
        else if (tok.type == ID)
        {
            int relative = 0;
            if (lb >= 146 && ub <= 150) /* JRs */
                relative = 1;
            val = sym_request(pass, tok.str, relative);
        }

        if (!filter(&lb, &ub, ccol, '%'))
        {
            char nbuf[16];
            ub = oub;
            lb = olb;
            if (filter(&lb, &ub, ccol, '?'))
            {
                sprintf(nbuf, "%02X", tok.num_val);
                for (i = 0; i < strlen(nbuf) + 1; i++)
                {
                    if (!filter(&lb, &ub, ccol + i + 1, nbuf[i]))
                    {
                        err(E, "invalid argument");
                        return;
                    }
                }
            }
            else
            {
                err(E, "invalid argument");
                return;
            }
        }
        else
        {
            if (opcodes[lb].str[ccol+1] == 'b')
            {
                if (val < -128 || val > 255)
                {
                    err(E, "constant too big");
                    return;
                }
            }
            else if (val < -32768 || val > 65535)
            {
                err(E, "constant too big");
                return;
            }
        }
    }
    else if (tok.type == KEYW)
    {
        for (i = 0; i < strlen(tok.str) + 1; ++i)
        {
            char c = toupper(tok.str[i]);
            if (!filter(&lb, &ub, ccol + i, c))
            {
                err(E, "invalid argument");
                return;
            }
        }
    }
    else if (tok.type == EOL)
    {
        err(E, "expected argument");
        return;
    }
    else
    {
        err(E, "invalid argument");
        return;
    }

    if (exp)
    {
        get_token();
        if (tok.type != exp)
        {
            if (exp == ')')
                err(E, "expected ')'");
            else
                err(E, "expected ']'");
            return;
        }
        exp = 0;
    }

    if (lb == ub)
    {
        get_token();
        if (tok.type == EOL)
            add_opcode(pass, lb, val);
        else
            err(E, "unexpected argument");
    }
    else
    {
        ccerr(F, " ");
    }
}




/*========================================================================*//**
 * \todo .byte and .word in ram sections does not need a value and issue
 * a warning
 *//*=========================================================================*/
void parse_directive(int pass)
{
    if (tok.type == _BYTE || tok.type == _WORD)
    {
        token_type_t type = tok.type;
        do
        {
            get_token();
            if (tok.type != NUM)
            {
                if (tok.type == EOL)
                {
                    if (pass == READ_PASS)
                        err(W, "undefined value in ROM address space");
                }
                else
                {
                    err(E, "expected numeric constant");
                    return;
                }
            }

            if (type == _BYTE && (tok.num_val < -128 || tok.num_val > 255))
            {
                err(E, "constant too big");
                return;
            }
            else if (type == _WORD && (tok.num_val < -32767))
            {
                err(E, "constant too big");
                return;
            }

            if (!add_data(pass, (tok.num_val & 0xFF)))
                return;
            if (type == _WORD)
            {
                if (!add_data(pass, ((tok.num_val & 0xFF00) >> 8)))
                    return;
            }

            if (tok.type != EOL)
                get_token();
            if (tok.type == EOL)
                break;
            if (tok.type != ',')
            {
                err(E, "expected ',' or end of line");
                return;
            }
        } while(1);

    }
    else if (tok.type == _ASCII)
    {
    }
    else if (tok.type == _ORG)
    {
        int address;

        get_token();
        if (tok.type != NUM)
        {
            err(E, "expected numeric constant after \".org\" directive");
            return;
        }

        address = tok.num_val;

        get_token();
        if (tok.type != EOL)
        {
            err(E, "unexpected argument");
            return;
        }

        add_section(pass, org, address);
    }
 }




/*========================================================================*//**
 * Case insensitive alpha string comparison
 *//*=========================================================================*/
int compare(const char* str1, const char* str2)
{
    while (*str1 && (toupper(*str1) == toupper(*str2)))
    {
        ++str1;
        ++str2;
    }
    return (unsigned)*str1 - (unsigned)*str2;
}




/*========================================================================*//**
 * Reduce the range of candidate opcodes given the current bounds
 *
 * \param lb: lower bound index in the opcode table
 * \param ub: upper bound index in the opcode table
 * \param col: column of the character to examine in an opcode
 * \param c: the character to compare
 *//*=========================================================================*/
int filter(int *lb, int *ub, int col, char c)
{
    while (opcodes[*lb].str[col] != c)
    {
        if (!c && opcodes[*lb].str[col] == ' ')
            break;

        ++*lb;
        if (*lb > *ub)
            return 0;
    }

    while (opcodes[*ub].str[col] != c)
    {
        if (!c && opcodes[*ub].str[col] == ' ')
            break;

        --*ub;
    }

    return 1;
}

/**
 * \} gbas
 */
