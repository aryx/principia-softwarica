/*s: db/input.c */
/*
 *
 *	debugger
 *
 */

#include "defs.h"
#include "fns.h"

/*s: global line */
Rune	line[LINSIZ];
/*e: global line */
extern	int	infile;
/*s: global lp */
Rune	*lp;
/*e: global lp */
int	peekc,lastc = EOR;
/*s: global eof */
int	eof;
/*e: global eof */

/*s: function eol */
/* input routines */

int
eol(int c)
{
    return(c==EOR || c==';');
}
/*e: function eol */

/*s: function rdc */
int
rdc(void)
{
    do {
        readchar();
    } while (lastc==SPC || lastc==TB);
    return(lastc);
}
/*e: function rdc */

/*s: function reread */
void
reread(void)
{
    peekc = lastc;
}
/*e: function reread */

/*s: function clrinp */
void
clrinp(void)
{
    flush();
    lp = 0;
    peekc = 0;
}
/*e: function clrinp */

/*s: function readrune */
int
readrune(int fd, Rune *r)
{
    char buf[UTFmax+1];
    int i;

    for(i=0; i<UTFmax && !fullrune(buf, i); i++)
        if(read(fd, buf+i, 1) <= 0)
            return -1;
    buf[i] = 0;
    chartorune(r, buf);
    return 1;
}
/*e: function readrune */

/*s: function readchar */
int
readchar(void)
{
    Rune *p;

    if (eof)
        lastc=0;
    else if (peekc) {
        lastc = peekc;
        peekc = 0;
    }
    else {
        if (lp==0) {
            for (p = line; p < &line[LINSIZ-1]; p++) {
                eof = readrune(infile, p) <= 0;
                if (mkfault) {
                    eof = 0;
                    error(0);
                }
                if (eof) {
                    p--;
                    break;
                }
                if (*p == EOR) {
                    if (p <= line)
                        break;
                    if (p[-1] != '\\')
                        break;
                    p -= 2;
                }
            }
            p[1] = 0;
            lp = line;
        }
        if ((lastc = *lp) != 0)
            lp++;
    }
    return(lastc);
}
/*e: function readchar */

/*s: function nextchar */
int
nextchar(void)
{
    if (eol(rdc())) {
        reread();
        return(0);
    }
    return(lastc);
}
/*e: function nextchar */

/*s: function quotchar */
int
quotchar(void)
{
    if (readchar()=='\\')
        return(readchar());
    else if (lastc=='\'')
        return(0);
    else
        return(lastc);
}
/*e: function quotchar */

/*s: function getformat */
void
getformat(char *deformat)
{
    char *fptr;
    BOOL	quote;
    Rune r;

    fptr=deformat;
    quote=FALSE;
    while ((quote ? readchar()!=EOR : !eol(readchar()))){
        r = lastc;
        fptr += runetochar(fptr, &r);
        if (lastc == '"')
            quote = ~quote;
    }
    lp--;
    if (fptr!=deformat)
        *fptr = '\0';
}
/*e: function getformat */

/*s: function isfileref */
/*
 *	check if the input line if of the form:
 *		<filename>:<digits><verb> ...
 *
 *	we handle this case specially because we have to look ahead
 *	at the token after the colon to decide if it is a file reference
 *	or a colon-command with a symbol name prefix. 
 */

int
isfileref(void)
{
    Rune *cp;

    for (cp = lp-1; *cp && !strchr(CMD_VERBS, *cp); cp++)
        if (*cp == '\\' && cp[1])	/* escape next char */
            cp++;
    if (*cp && cp > lp-1) {
        while (*cp == ' ' || *cp == '\t')
            cp++;
        if (*cp++ == ':') {
            while (*cp == ' ' || *cp == '\t')
                cp++;
            if (isdigit(*cp))
                return 1;
        }
    }
    return 0;
}
/*e: function isfileref */
/*e: db/input.c */
