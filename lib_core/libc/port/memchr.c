/*s: port/memchr.c */
#include    <u.h>
#include    <libc.h>

/*s: function memchr */
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
/*e: function memchr */
/*e: port/memchr.c */
