/*s: port/lrand.c */
#include    <u.h>
#include    <libc.h>

/*s: constant LEN */
/*
 *  algorithm by
 *  D. P. Mitchell & J. A. Reeds
 */

#define LEN 607
/*e: constant LEN */
/*s: constant TAP */
#define TAP 273
/*e: constant TAP */
/*s: constant MASK (port/lrand.c) */
#define MASK    0x7fffffffL
/*e: constant MASK (port/lrand.c) */
/*s: constant A */
#define A   48271
/*e: constant A */
/*s: constant M */
#define M   2147483647
/*e: constant M */
/*s: constant Q */
#define Q   44488
/*e: constant Q */
/*s: constant R */
#define R   3399
/*e: constant R */
/*s: constant NORM (port/lrand.c) */
#define NORM    (1.0/(1.0+MASK))
/*e: constant NORM (port/lrand.c) */

/*s: global rng_vec */
static  ulong   rng_vec[LEN];
/*e: global rng_vec */
/*s: global rng_tap */
static  ulong*  rng_tap = rng_vec;
/*e: global rng_tap */
/*s: global rng_feed */
static  ulong*  rng_feed = 0;
/*e: global rng_feed */
/*s: global lk */
static  Lock    lk;
/*e: global lk */

/*s: function isrand */
static void
isrand(long seed)
{
    long lo, hi, x;
    int i;

    rng_tap = rng_vec;
    rng_feed = rng_vec+LEN-TAP;
    seed = seed%M;
    if(seed < 0)
        seed += M;
    if(seed == 0)
        seed = 89482311;
    x = seed;
    /*
     *  Initialize by x[n+1] = 48271 * x[n] mod (2**31 - 1)
     */
    for(i = -20; i < LEN; i++) {
        hi = x / Q;
        lo = x % Q;
        x = A*lo - R*hi;
        if(x < 0)
            x += M;
        if(i >= 0)
            rng_vec[i] = x;
    }
}
/*e: function isrand */

/*s: function srand */
void
srand(long seed)
{
    lock(&lk);
    isrand(seed);
    unlock(&lk);
}
/*e: function srand */

/*s: function lrand */
long
lrand(void)
{
    ulong x;

    lock(&lk);

    rng_tap--;
    if(rng_tap < rng_vec) {
        if(rng_feed == 0) {
            isrand(1);
            rng_tap--;
        }
        rng_tap += LEN;
    }
    rng_feed--;
    if(rng_feed < rng_vec)
        rng_feed += LEN;
    x = (*rng_feed + *rng_tap) & MASK;
    *rng_feed = x;

    unlock(&lk);

    return x;
}
/*e: function lrand */
/*e: port/lrand.c */
