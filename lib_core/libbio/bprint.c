/*s: libbio/bprint.c */
/*s: libbio includes */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>
/*e: libbio includes */
/*s: function [[Bprint]] */
int
Bprint(Biobufhdr *bp, char *fmt, ...)
{
    va_list arg;
    int n;

    va_start(arg, fmt);
    n = Bvprint(bp, fmt, arg);
    va_end(arg);
    return n;
}
/*e: function [[Bprint]] */
/*e: libbio/bprint.c */
