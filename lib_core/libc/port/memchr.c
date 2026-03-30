/*s: libc/port/memchr.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[memchr]] */
void*
memchr(void *ap, int c, ulong n)
{
    uchar *sp;

    sp = ap;
    c &= 0xFF;
    while(n > 0) {
        if(*sp++ == c)
            return sp-1;
        n--;
    }
    return nil;
}
/*e: function [[memchr]] */
/*e: libc/port/memchr.c */
