/*s: libc/port/lnrand.c */
#include    <u.h>
#include    <libc.h>

/*s: constant MASK (port/lnrand.c) */
#define MASK    0x7fffffffL
/*e: constant MASK (port/lnrand.c) */

/*s: function [[lnrand]] */
long
lnrand(long n)
{
    long slop, v;

    if(n < 0)
        return n;
    slop = MASK % n;
    do
        v = lrand();
    while(v <= slop);
    return v % n;
}
/*e: function [[lnrand]] */
/*e: libc/port/lnrand.c */
