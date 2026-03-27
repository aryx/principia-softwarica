/*s: libstring/s_putc.c */
/*s: libstring includes */
#include <u.h>
#include <libc.h>
#include <str.h> // was string.h
/*e: libstring includes */

/*s: function [[s_putc]] */
void
s_putc(String *s, int c)
{
    if(s->ref > 1)
        sysfatal("can't s_putc a shared string");
    if (s->ptr >= s->end)
        s_grow(s, 2);
    *(s->ptr)++ = c;
}
/*e: function [[s_putc]] */
/*e: libstring/s_putc.c */
