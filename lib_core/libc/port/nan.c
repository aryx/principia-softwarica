/*s: port/nan.c */
#include <u.h>
#include <libc.h>

/*s: constant NANEXP */
#define NANEXP  (2047<<20)
/*e: constant NANEXP */
/*s: constant NANMASK */
#define NANMASK (2047<<20)
/*e: constant NANMASK */
/*s: constant NANSIGN */
#define NANSIGN (1<<31)
/*e: constant NANSIGN */

/*s: function NaN */
double
NaN(void)
{
    FPdbleword a;

    a.hi = NANEXP;
    a.lo = 1;
    return a.x;
}
/*e: function NaN */

/*s: function isNaN */
int
isNaN(double d)
{
    FPdbleword a;

    a.x = d;
    if((a.hi & NANMASK) != NANEXP)
        return 0;
    return !isInf(d, 0);
}
/*e: function isNaN */

/*s: function Inf */
double
Inf(int sign)
{
    FPdbleword a;

    a.hi = NANEXP;
    a.lo = 0;
    if(sign < 0)
        a.hi |= NANSIGN;
    return a.x;
}
/*e: function Inf */

/*s: function isInf */
int
isInf(double d, int sign)
{
    FPdbleword a;

    a.x = d;
    if(a.lo != 0)
        return 0;
    if(a.hi == NANEXP)
        return sign >= 0;
    if(a.hi == (NANEXP|NANSIGN))
        return sign <= 0;
    return 0;
}
/*e: function isInf */
/*e: port/nan.c */
