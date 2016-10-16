/*s: port/floor.c */
#include <u.h>
#include <libc.h>
/*s: function floor */
/*
 * floor and ceil-- greatest integer <= arg
 * (resp least >=)
 */

double
floor(double d)
{
    double fract;

    if(d < 0) {
        fract = modf(-d, &d);
        if(fract != 0.0)
            d += 1;
        d = -d;
    } else
        modf(d, &d);
    return d;
}
/*e: function floor */

/*s: function ceil */
double
ceil(double d)
{
    return -floor(-d);
}
/*e: function ceil */
/*e: port/floor.c */
