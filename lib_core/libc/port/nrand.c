/*s: libc/port/nrand.c */
#include    <u.h>
#include    <libc.h>

/*s: constant MASK (port/nrand.c) */
#define MASK    0x7fffffffL
/*e: constant MASK (port/nrand.c) */

/*s: function [[nrand]] */
int
nrand(int n)
{
    long slop, v;

    if(n < 0)
        return n;
    if(n == 1)
        return 0;
    /* and if n == 0, you deserve what you get */
    slop = MASK % n;
    do
        v = lrand();
    while(v <= slop);
    return v % n;
}
/*e: function [[nrand]] */
/*e: libc/port/nrand.c */
