/*s: libc/port/rand.c */
#include    <u.h>
#include    <libc.h>

/*s: function [[rand]] */
int
rand(void)
{
    return lrand() & 0x7fff;
}
/*e: function [[rand]] */
/*e: libc/port/rand.c */
