/*s: machine/5i/utils.c */
/*s: basic includes */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

#include "arm.h"
/*e: basic includes */

/*s: function [[fatal]] */
void
fatal(bool syserr, char *fmt, ...)
{
    char buf[ERRMAX], *s;
    va_list arg;

    va_start(arg, fmt);
    vseprint(buf, buf+sizeof(buf), fmt, arg);
    va_end(arg);

    s = "5i: %s\n";
    if(syserr)
        s = "5i: %s: %r\n";
    fprint(STDERR, s, buf);
    exits(buf);
}
/*e: function [[fatal]] */

/*s: function [[itrace]] */
void
itrace(char *fmt, ...)
{
    char buf[128];
    va_list arg;

    va_start(arg, fmt);
    vseprint(buf, buf+sizeof(buf), fmt, arg);
    va_end(arg);

    Bprint(bout, "%8lux %.8lux %2d %s\n", 
                     reg.ar, reg.instr, reg.instr_opcode, buf);	
    Bflush(bout);
}
/*e: function [[itrace]] */

/*s: function [[dumpreg]] */
void
dumpreg(void)
{
    int i;

    Bprint(bout, "PC  #%-8lux SP  #%-8lux \n",
                reg.r[REGPC], reg.r[REGSP]);

    for(i = 0; i < 16; i++) {
        if((i%4) == 0 && i != 0)
            Bprint(bout, "\n");
        Bprint(bout, "R%-2d #%-8lux ", i, reg.r[i]);
    }
    Bprint(bout, "\n");
}
/*e: function [[dumpreg]] */

/*s: function [[dumpfreg]] */
void
dumpfreg(void)
{
}
/*e: function [[dumpfreg]] */

/*s: function [[dumpdreg]] */
void
dumpdreg(void)
{
}
/*e: function [[dumpdreg]] */

/*s: function [[emalloc]] */
void *
emalloc(ulong size)
{
    void *a;

    a = malloc(size);
    if(a == nil)
        fatal(false, "no memory");

    memset(a, 0, size); //!!
    return a;
}
/*e: function [[emalloc]] */

/*s: function [[erealloc]] */
void *
erealloc(void *a, ulong oldsize, ulong size)
{
    void *n;

    n = malloc(size);
    if(n == nil)
        fatal(false, "no memory");
    memset(n, 0, size);
    if(size > oldsize)
        size = oldsize;
    memmove(n, a, size);
    return n;
}
/*e: function [[erealloc]] */
/*e: machine/5i/utils.c */
