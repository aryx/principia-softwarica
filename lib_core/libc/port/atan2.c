/*s: port/atan2.c */
#include <u.h>
#include <libc.h>

/*s: function atan2 */
/*
    atan2 discovers what quadrant the angle
    is in and calls atan.
*/

double
atan2(double arg1, double arg2)
{

    if(arg1+arg2 == arg1) {
        if(arg1 >= 0)
            return PIO2;
        return -PIO2;
    }
    arg1 = atan(arg1/arg2);
    if(arg2 < 0) {
        if(arg1 <= 0)
            return arg1 + PI;
        return arg1 - PI;
    }
    return arg1;
}
/*e: function atan2 */
/*e: port/atan2.c */
