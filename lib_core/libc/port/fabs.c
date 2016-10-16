/*s: port/fabs.c */
#include <u.h>
#include <libc.h>

/*s: function fabs */
double
fabs(double arg)
{

    if(arg < 0)
        return -arg;
    return arg;
}
/*e: function fabs */
/*e: port/fabs.c */
