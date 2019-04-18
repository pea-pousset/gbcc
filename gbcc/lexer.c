/*
Handled:
--------
[a-zA-z_]([a-zA-z0-9_])*            IDENTIFIER
[1-9]([0-9])*(u|U)                  CONSTANT (decimal)
0([0-7])*(u|U)                      CONSTANT (octal)
0x[0-9a-fA-F]([0-9a-fA-F])*(u|U)    CONSTANT (hex)

( ) [ ] { } ; ,
! =
- + * / %
> < >= <= == !=

byte
word

return

================================================================================

NOT handled:
------------
auto
break
case
char
const
continue
default
do
double
else
enum
extern
float
for
goto
if
int
long
register
short
signed
sizeof
static
struct
switch
typedef
union
unsigned
void
volatile
while

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

L?'(\\.|[^\\'])+'	{ count(); return(CONSTANT); }

{D}+{E}{FS}?		{ count(); return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ count(); return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ count(); return(CONSTANT); }

L?\"(\\.|[^\\"])*\"	{ count(); return(STRING_LITERAL); }

...
>>=
<<=
+=
-=
*=
/=
%=
&=
^=
|=
>>
<<
++
--
->
&&
||


:
.
&
!
~

^
|
?
 */

/**
 * \addtogroup gbcc
 * \{
 * \defgroup Lexer
 * \addtogroup Lexer
 * \{
 */

#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "../common/options.h"
#include "../common/errors.h"

static FILE* f;          /**< Current file */
static int      c;       /**< Current character */
static int      l;       /**< Look-ahead character */
static int      prev;    /**< Previous character */
static unsigned line;    /**< Current line number in the source file */
static unsigned column;  /**< Current column number in the source file */

static int      bc;      /**< Current character backup */
static int      bl;      /**< Current look-ahead character backup */
static int      bprev;   /**< Previous character backup */
static unsigned bline;   /**< Current line number backup */
static unsigned bcolumn; /**< Current column backup */
static unsigned bseek;   /**< Current position in the file backup */
static int      tabstop; /**< Width of the tab character */

static token_t t;        /**< Current token */

static void  readbyte();
static int   get_digit(int base);
static void  parse_num();
static char* pp_get_token();
static void  parse_pp_info();

/*========================================================================*//**
 * Initializes the lexer for a new file
 *//*=========================================================================*/
void lexer_init(FILE* infile)
{
    f = infile;
    tabstop = get_option("-ftabstop=")->value.num;

    readbyte();

    /* skip UTF-8 BOM */
    if (l == 0xEF)
    {
        readbyte();
        if (l == 0xBB)
        {
            readbyte();
            if (l == 0xBF)
                readbyte();
            else
            {
                fseek(f, 1, SEEK_SET);
                l = 0xEF;
            }
        }
        else
        {
            fseek(f, 1, SEEK_SET);
            l = 0xEF;
        }
    }

    line = 1;
    column = 0;
    c = '\n';
}

/*========================================================================*//**
 * Get the next token
 *//*=========================================================================*/
token_t* get_token()
{
    int skip = '1';

    readbyte();

    while (skip)
    {
        while (isspace(c))
            readbyte();

        if (c == '/' && (l == '*' || l == '/'))
        {
            eline = line;
            ecolumn = column;

            readbyte();
            if (c == '*')
            {
                do
                {
                    readbyte();
                    if (c == EOF)
                    {
                        err(E, "unterminated comment");
                        break;
                    }
                } while (!(c == '*' && l == '/'));
                readbyte();
                readbyte();
            }
            else
            {
                do
                {
                    readbyte();
                } while (c != '\n');
            }
        }
        else
            skip = 0;
    }

    eline = line;
    ecolumn = column;

    t.id = c;
    t.line = line;
    t.column = column;

    if (c == '#' && prev == '\n' && isspace(l))
    {
        parse_pp_info();
        return get_token();
    }

    if (c == EOF)
    {
    }
    else if (isalpha(c) || c == '_')
    {
        char* p = t.string;
        int len = 1;
        t.id = IDENTIFIER;

        *p++ = c;
        while (isalnum(l) || l == '_')
        {
            readbyte();
            if (len < MAX_ID_LEN)
            {
                *p++ = c;
                ++len;
            }
        }
        *p = 0;

        if (len >= MAX_ID_LEN)
        {
            err(W, "too long identifier has been truncated to %d characters.",
                 MAX_ID_LEN);
        }

        if (strcmp(t.string, "byte") == 0)
            t.id = BYTE;
        else if (strcmp(t.string, "return") == 0)
            t.id = RETURN;
        else if (strcmp(t.string, "void") == 0)
            t.id = VOID;
        else if (strcmp(t.string, "word") == 0)
            t.id = WORD;
    }
    else if (isdigit(c))
    {
        t.id = CONSTANT;
        t._unsigned = 0;
        t._word = 0;
        parse_num();
    }
    else if ( c == '>')
    {
        if (l == '=')
        {
            readbyte();
            t.id = GE_OP;
        }
    }
    else if (c == '<')
    {
        if (l == '=')
        {
            readbyte();
            t.id = LE_OP;
        }
    }
    else if (c == '!')
    {
        if (l == '=')
        {
            readbyte();
            t.id = NE_OP;
        }
    }
    else if (c == '=')
    {
        if (l == '=')
        {
            readbyte();
            t.id = EQ_OP;
        }
    }
    else if (c == '-')
    {
    }
    else if (c == '+')
    {
    }
    else if (c == '*')
    {
    }
    else if (c == '/')
    {
    }
    else if (c == '%')
    {
    }
    else if (   c == '(' || c == ')'
             || c == '{' || c == '}'
             || c == '[' || c == ']'
             || c == ';' || c == ',')
    {
    }
    else
    {
        if (c < ' ' || c >= 127)
            err(E, "stray '%o' in program", c);
        else
            err(E, "stray \"%c\" in program", c);
        return get_token();
    }

    return &t;
}

/*========================================================================*//**
 *
 *//*=========================================================================*/
const char* token_name(int id)
{
    static char single[] = "' '";

    switch (id)
    {
        case EOF: return "end of file";

        case BYTE: return "'byte'";
        case ELSE: return "'else'";
        case IF:   return "'if'";
        case RETURN: return "'return'";
        case VOID: return "'void'";
        case WORD: return "'word'";

        case CONSTANT: return "numeric constant";
        case IDENTIFIER: return "identifier";

        case EQ_OP: return "'=='";
        case NE_OP: return "'!='";
        case LE_OP: return "'<='";
        case GE_OP: return "'>='";

        default:
            single[1] = id;
            return single;
    }
}

void lexer_backtrace_prepare()
{
    bc = c;
    bl = l;
    bprev = prev;
    bline = line;
    bcolumn = column;
    bseek = ftell(f);
}

void lexer_backtrace()
{
    c = bc;
    l = bl;
    prev = bprev;
    line = bline;
    column = bcolumn;
    fseek(f, bseek, SEEK_SET);
}

void token_copy(token_t* from, token_t* to)
{
    to->id = from->id;
    to->line = from->line;
    to->column = from->column;
    to->constant = from->constant;
    to->_unsigned = from->_unsigned;
    to->_word = from->_word;
    strcpy(to->string, from->string);
}

/*========================================================================*//**
 * Read a character from the source file and update the line and column
 * numbers. All different line ending formats are handled and return a single
 * '\n'.
 *//*=========================================================================*/
void readbyte()
{
    prev = c;
    c = l;
    l = fgetc(f);

    if (c == '\n')
    {
        if (l == '\r')
        {
            prev = c;
            c = '\n';
            l = fgetc(f);
        }
        ++line;
        column = 0;
    }
    else if (c == '\r')
    {
        if (l == '\n')
        {
            prev = c;
            c = l;
            l = fgetc(f);
        }
        ++line;
        column = 0;
        c = '\n';
    }
    else if (c == '\t')
    {
        column += tabstop - (column % tabstop);
    }
    else if (c == '\v')
    {
        line += 8 - (line % 8);
        column = 0;
        c = '\n';
    }
    else
        ++column;
}

/*========================================================================*//**
 * If available, read a digit in base 'base' and convert it to a
 * decimal factor ; if no more digit are available, return -1
 *//*=========================================================================*/
int get_digit(int base)
{
    if (base == 10)
    {
        if (isdigit(l))
        {
            readbyte();
            return (c - '0');
        }
    }
    else if (base == 16)
    {
        if (isdigit(l))
        {
            readbyte();
            return (c - '0');
        }
        else if (l >= 'a' && l <= 'f')
        {
            readbyte();
            return (c - 'a' + 10);
        }
        else if (l >= 'A' && l <= 'F')
        {
            readbyte();
            return (c - 'A' + 10);
        }
    }
    else if (base == 8)
    {
        /* accept all digits so we don't have to treat non octal digits as
        a suffix */
        if (isdigit(l))
        {

            readbyte();
            if (c >= '0' && c <= '7')
            {
                return (c - '0');
            }
            else
            {
                /* backup of error line and column because the parser assumes
                that they are set to the start of the current token and does not
                update them */
                int bl = eline;
                int bc = ecolumn;
                err(E, "invalid digit \"%c\" in octal constant", c);
                eline = bl;
                ecolumn = bc;
            }
        }
    }

    return -1;
}

/*========================================================================*//**
 * Parse a constant number literal
 *//*=========================================================================*/
void parse_num()
{
    int base = 10;
    int digit;
    int overflow_8 = 0;
    int overflow_16 = 0;

    if (c == '0')
    {
        t.constant = 0;
        base = 8;

        if (l == 'x' || l == 'X')
        {
            int bl = line;
            int bc = column;
            long fp = ftell(f);

            base = 16;
            readbyte();

            if ( ! (isdigit(l)
                   || (l >= 'a' && l <= 'f')
                   || (l >= 'A' && l <= 'F'))
               )
            {
                fseek(f, fp, SEEK_SET);
                l = c;
                line = bl;
                column = bc;
            }
        }
    }
    else
        t.constant = c - '0';

    while ( (digit = get_digit(base)) != -1 )
    {
        t.constant *= base;
        t.constant += digit;

        /* If the constant does not fit into an unsigned int and overflows,
        the value may later seem to fit into our "byte" or "word" */
        if (t.constant > 0xFF)
            overflow_8 = 1;
        if (t.constant > 0xFFFF)
            overflow_16 = 1;
    }

    /* eat up all alpha/underscore characters as a part of the suffix in order
     to not treat them as an identifier */
    if (isalpha(l) || l == '_')
    {
        char suffix[9];
        char* p = suffix;
        int len = 1;
        int sign_found = 0;     /* U suffix */
        int width_found = 0;    /* W suffix */

        readbyte();
        *p++ = c;
        while (isalnum(l))
        {
            readbyte();
            if (len < 8)
            {
                *p++ = c;
                ++len;
            }
        }
        *p = 0;

        p = suffix;
        while (*p)
        {
            if (*p == 'u' || *p == 'U')
            {
                if (sign_found)
                    goto suffix_error;
                t._unsigned = 1;
                sign_found = 1;
            }
            else if (*p == 'w' || *p == 'W')
            {
                if (width_found)
                    goto suffix_error;
                t._word = 1;
                width_found = 1;
            }
            else
                goto suffix_error;

            ++p;
            continue;

        suffix_error:
            err(E, "invalid suffix \"%s%s\" after integer constant", suffix, len == 8 ? "..." : "");
            return;
        }
    }

    if (!t._word)
    {
        if (!t._unsigned && t.constant > 127)
            t._unsigned = 1;

        if (overflow_8)
        {
            t._unsigned = 1;
            t.constant &= 0xFF;
            err(W, "numeric constant is too large for its type");
        }
    }
    else
    {
        if (!t._unsigned && t.constant > 32767)
            t._unsigned = 1;

        if (overflow_16)
        {
            t._unsigned = 1;
            t.constant &= 0xFFFF;
            err(W, "numeric constant is too large for its type");
        }
    }



}

/*========================================================================*//**
 *
 *//*=========================================================================*/
char* pp_get_token()
{
    static char buf[1024];
    int len = 1;
    int termchar = 0;
    char* p = buf;

    /* end of line reached */
    if (l == '\n' || l == EOF)
        return NULL;

    /* eat up last token's last character */
    readbyte();

    /* skip spaces */
    while (l != '\n' && l != EOF && isspace(c))
        readbyte();

    /* no token */
    if (isspace(c) || c == EOF)
        return NULL;

    eline = line;
    ecolumn = column;

    if (c == '"')
        termchar = '"';

    len = 1;
    *p++ = c;
    while (l != EOF)
    {
        if (l == '\n')
            break;

        if (!termchar && isspace(l))
            break;

        readbyte();
        if (len < 1023)
        {
            *p++ = c;
            ++len;
        }

        if (termchar && c == termchar)
            break;
    }
    *p = 0;

    if (len >= 1023)
        ccerr(F, "too long line directive");

    return buf;
}

/*========================================================================*//**
 *
 *//*=========================================================================*/
void parse_pp_info()
{
    char* p;
    char* p2;
    int newline;
    char newfile[1024];

    /* get line number token */
    if (!(p = pp_get_token()))
        return;

    /* verify format */
    p2 = p;
    while (*p2)
    {
        if (!isdigit(*p2))
        {
            while (l != '\n' && l != EOF)
                readbyte();
            err(E, "invalid line number \"%s\"", p);
            return;
        }
        ++p2;
    }

    newline = atoi(p) - 1;

    /* get file name token */
    if (!(p = pp_get_token()))
    {
        line = newline;
        return;
    }

    /* verify format */
    p2 = p;
    while (*p2)
        ++p2;
    --p2;
    if (*p != '"' || *p2 != '"')
    {
        err(E, "\"%s\" is not a valid filename", p);

        while (l != '\n' && l != EOF)
            readbyte();
        line = newline;
        return;
    }

    ++p;
    strcpy(newfile, p);
    newfile[strlen(p) - 1] = 0;

    /* check for unwanted tokens */
    if ((p = pp_get_token()))
    {
        err(E, "unexpected flag \"%s\" in line directive", p);

        while (l != '\n' && l != EOF)
            readbyte();


        line = newline;
        esetfile(newfile);
        return;
    }

    esetfile(newfile);
    line = newline;
}

/**
 * \} Lexer
 * \} gbcc
 */
