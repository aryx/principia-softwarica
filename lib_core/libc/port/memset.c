/*s: libc/port/memset.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[memset]] */
void*
memset(void *ap, int c, ulong n)
{
    char *p;

    p = ap;
    while(n > 0) {
        *p++ = c;
        n--;
    }
    return ap;
}
/*e: function [[memset]] */
/*e: libc/port/memset.c */
