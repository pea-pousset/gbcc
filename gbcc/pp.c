#include "pp.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../common/errors.h"
#include "../common/options.h"

#define BUFSIZE 4096

static const char* filnam; /* to be changed for an include stack */

static FILE* f;
static int c;
static int l;
static char linebuf[BUFSIZE];   /* current line buffer */
static int  mlc;                /* multi line comment */
static int line;
static int column;
static int update_line;         /* output a " # line-num file" when set */ 
static int tabstop;

static void readbyte();
static int  readline();
static void process();

/**
 * \param infile: source file in binary read mode
 * \param outfile: output preprocessed source file in binary write mode
 */
int pp(const char* filename, FILE* infile, FILE* outfile)
{
    unsigned char bom1, bom2, bom3;
    char* pl;
    int loop;
    
    tabstop = get_option("-ftabstop=")->value.num;
    f = infile;

    
    filnam = filename;

    /* accept UTF-8 BOM */
    bom1 = fgetc(f);
    bom2 = fgetc(f);
    bom3 = fgetc(f);
    if (!(bom1 == 0xEF && bom2 == 0xBB && bom3 == 0xBF))
        rewind(f);

    c = '\n';
    l = fgetc(f);
    mlc = 0;
    line = 1;

    fprintf(outfile, "# 1 \"%s\"\n", filename);
    do
    {
        int curline = line;
        loop = 0;
        pl = linebuf;
        *pl = 0;
        column = 0;    
        update_line = 0;
        
        loop = readline(pl);  
        ++line;  
        
        pl = linebuf;
        while (*pl)
            fputc(*pl++, outfile);
            
        if (update_line)
            fprintf(outfile, "# %d \"%s\"\n", line, filename);
            
        process(curline);
            
     
    } while (loop);

    return 1;
}

void readbyte()
{
    c = l;
    l = fgetc(f);
}

int readline(char* pl)
{
    if (c == EOF)
        return 0;

    while (c != EOF)
    {
        readbyte();

        if (c == EOF)
        {
            if (mlc)
                err(E, "unterminated comment");

        }
        else if (c == '\n')
        {
            if (l == '\r')
                readbyte();

            *pl++ = '\n';
            break;
        }
        else if (c == '\r')
        {
            if (l == '\n')
                readbyte();

            *pl++ = '\n';
            break;
        }
        else if (c == '\t')
        {
            int sp = tabstop - (column % tabstop);
            column += sp;
            while (sp--)
                *pl++ = ' ';
        }
        /*else if (c == '\v')
        {

        } */
        else if (!mlc && c == '/' && l == '/')
        {
            while (l != '\n' && l != '\r' && l != '\v')
                readbyte();
        }
        else if (!mlc && c == '/' && l == '*')
        {
            readbyte();
            *pl++ = ' ';
            *pl++ = ' ';
            mlc = 1;
        }
        else if (mlc && c == '*' && l == '/')
        {
            readbyte();
            *pl++ = ' ';
            *pl++ = ' ';
            mlc = 0;

            eline = line;
            ecolumn = column;
        }
        else
        {
            ++column;
            if (mlc)
                *pl++ = ' ';
            else
                *pl++ = c;
        }

        if (column >= BUFSIZE)
            ccerr(F, "line too long in file \"%s\" at line %d", filnam, line);
    }
    *pl = 0;
    
    pl = linebuf;
    while (isspace(*pl))
        pl++;
    
    /* empty line ? */
    if (!*pl)
    {
        linebuf[0] = '\n';
        linebuf[1] = 0;
        return 1;
    }
    
    /* search last non white character */
    while (*pl)
        ++pl;
    --pl;
    
    while (isspace(*pl))
    {
        --pl;
        --column;
    }
    
    /* merge lines or remove trailing spaces */
    if (*pl == '\\')
    {
        ++line;
        readline(pl);
        update_line = 1;
    }
    else
    {
        --column;
        *++pl = '\n';
        *++pl = 0;
    }
    
    return 1;
}

void process(int linenum)
{
    
}
