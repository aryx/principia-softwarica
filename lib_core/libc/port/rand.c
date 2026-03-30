/*s: libc/port/rand.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[rand]] */
int
rand(void)
{
    return lrand() & 0x7fff;
}
/*e: function [[rand]] */
/*e: libc/port/rand.c */
