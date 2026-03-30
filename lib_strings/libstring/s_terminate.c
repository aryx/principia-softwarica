/*s: libstring/s_terminate.c */
/*s: libstring includes */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
#include <str.h> // was string.h
/*e: libstring includes */

/*s: function [[s_terminate]] */
void
s_terminate(String *s)
{
    if(s->ref > 1)
        sysfatal("can't s_terminate a shared string");
    if (s->ptr >= s->end)
        s_grow(s, 1);
    *s->ptr = 0;
}
/*e: function [[s_terminate]] */
/*e: libstring/s_terminate.c */
