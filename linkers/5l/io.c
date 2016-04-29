/*s: linkers/5l/io.c */
#include	"l.h"

/*s: function readsome */
static byte*
readsome(fdt f, byte *buf, byte *good, byte *stop, int max)
{
    int n;

    n = stop - good;
    memmove(buf, good, n);
    stop = buf + n;
    n = MAXIO - n;
    if(n > max)
        n = max;
    n = read(f, stop, n);
    if(n <= 0)
        return nil;
    return stop + n;
}
/*e: function readsome */


/*s: function strnput(arm) */
void
strnput(char *s, int n)
{
    for(; *s; s++){
        cput(*s);
        n--;
    }
    for(; n > 0; n--)
        cput(0);
}
/*e: function strnput(arm) */

/*s: function cput(arm) */
void
cput(int c)
{
    cbp[0] = c;
    cbp++;
    cbc--;
    if(cbc <= 0)
        cflush();
}
/*e: function cput(arm) */

/*s: function wput(arm) */
void
wput(long l)
{

    cbp[0] = l>>8;
    cbp[1] = l;
    cbp += 2;
    cbc -= 2;
    if(cbc <= 0)
        cflush();
}
/*e: function wput(arm) */

/*s: function wputl(arm) */
void
wputl(long l)
{

    cbp[0] = l;
    cbp[1] = l>>8;
    cbp += 2;
    cbc -= 2;
    if(cbc <= 0)
        cflush();
}
/*e: function wputl(arm) */

/*s: function lput(arm) */
void
lput(long l)
{

    cbp[0] = l>>24;
    cbp[1] = l>>16;
    cbp[2] = l>>8;
    cbp[3] = l;
    cbp += 4;
    cbc -= 4;
    if(cbc <= 0)
        cflush();
}
/*e: function lput(arm) */

/*s: function lputl(arm) */
void
lputl(long l)
{

    cbp[3] = l>>24;
    cbp[2] = l>>16;
    cbp[1] = l>>8;
    cbp[0] = l;
    cbp += 4;
    cbc -= 4;
    if(cbc <= 0)
        cflush();
}
/*e: function lputl(arm) */

/*s: function cflush */
void
cflush(void)
{
    int n;

    n = sizeof(buf.obuf) - cbc;
    if(n)
        write(cout, buf.obuf, n);

    cbp = buf.obuf;
    cbc = sizeof(buf.obuf);
}
/*e: function cflush */

/*e: linkers/5l/io.c */
