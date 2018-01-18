/*s: port/frand.c */
#include    <u.h>
#include    <libc.h>

/*s: constant [[MASK]] */
#define MASK    0x7fffffffL
/*e: constant [[MASK]] */
/*s: constant [[NORM]] */
#define NORM    (1.0/(1.0+MASK))
/*e: constant [[NORM]] */

/*s: function [[frand]] */
double
frand(void)
{
    double x;

    do {
        x = lrand() * NORM;
        x = (x + lrand()) * NORM;
    } while(x >= 1);
    return x;
}
/*e: function [[frand]] */
/*e: port/frand.c */
