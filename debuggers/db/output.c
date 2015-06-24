/*s: db/output.c */
/*
 *
 *	debugger
 *
 */

#include "defs.h"
#include "fns.h"

/*s: global printcol */
int	printcol = 0;
/*e: global printcol */
/*s: global infile */
int	infile = STDIN;
/*e: global infile */
/*s: global maxpos */
int	maxpos = MAXPOS;
/*e: global maxpos */

/*s: global stdout */
Biobuf	stdout;
/*e: global stdout */

/*s: function printc */
void
printc(int c)
{
    dprint("%c", c);
}
/*e: function printc */

/*s: function tconv */
/* was move to next f1-sized tab stop; now just print a tab */
int
tconv(Fmt *f)
{
    return fmtstrcpy(f, "\t");
}
/*e: function tconv */

/*s: function flushbuf */
void
flushbuf(void)
{
  if (printcol != 0)
        printc(EOR);
}
/*e: function flushbuf */

/*s: function prints */
void
prints(char *s)
{
    dprint("%s",s);
}
/*e: function prints */

/*s: function newline */
void
newline(void)
{
    printc(EOR);
}
/*e: function newline */

/*s: constant MAXIFD */
#define	MAXIFD	5
/*e: constant MAXIFD */
/*s: global istack */
struct {
    int	fd;
    int	r9;
} istack[MAXIFD];
/*e: global istack */
/*s: global ifiledepth */
int	ifiledepth;
/*e: global ifiledepth */

/*s: function iclose */
void
iclose(int stack, int err)
{
    if (err) {
        if (infile) {
            close(infile);
            infile=STDIN;
        }
        while (--ifiledepth >= 0)
            if (istack[ifiledepth].fd)
                close(istack[ifiledepth].fd);
        ifiledepth = 0;
    } else if (stack == 0) {
        if (infile) {
            close(infile);
            infile=STDIN;
        }
    } else if (stack > 0) {
        if (ifiledepth >= MAXIFD)
            error("$<< nested too deeply");
        istack[ifiledepth].fd = infile;
        ifiledepth++;
        infile = STDIN;
    } else {
        if (infile) {
            close(infile); 
            infile=STDIN;
        }
        if (ifiledepth > 0) {
            infile = istack[--ifiledepth].fd;
        }
    }
}
/*e: function iclose */

/*s: function oclose */
void
oclose(void)
{
    flushbuf();
    Bterm(&stdout);
    Binit(&stdout, 1, OWRITE);
}
/*e: function oclose */

/*s: function redirout */
void
redirout(char *file)
{
    int fd;

    if (file == 0){
        oclose();
        return;
    }
    flushbuf();
    if ((fd = open(file, 1)) >= 0)
        seek(fd, 0L, 2);
    else if ((fd = create(file, 1, 0666)) < 0)
        error("cannot create");
    Bterm(&stdout);
    Binit(&stdout, fd, OWRITE);
}
/*e: function redirout */

/*s: function endline */
void
endline(void)
{

    if (printcol >= maxpos)
        newline();
}
/*e: function endline */

/*s: function flush */
void
flush(void)
{
    Bflush(&stdout);
}
/*e: function flush */

/*s: function dprint */
int
dprint(char *fmt, ...)
{
    int n, w;
    char *p;
    char buf[4096];
    Rune r;
    va_list arg;

    /*s: [[dprint()]] return if mkfault */
    if(mkfault)
        return -1;
    /*e: [[dprint()]] return if mkfault */

    va_start(arg, fmt);
    n = vseprint(buf, buf+sizeof buf, fmt, arg) - buf;
    va_end(arg);

    //Bprint(&stdout, "[%s]", fmt);
    Bwrite(&stdout, buf, n);

    /*s: [[dprint()]] maintain printcol */
    for(p=buf; *p; p+=w){
        w = chartorune(&r, p);
        if(r == '\n')
            printcol = 0;
        else
            printcol++;
    }
    /*e: [[dprint()]] maintain printcol */
    return n;
}
/*e: function dprint */

/*s: function outputinit */
void
outputinit(void)
{
    Binit(&stdout, 1, OWRITE);
    fmtinstall('t', tconv);
}
/*e: function outputinit */
/*e: db/output.c */
