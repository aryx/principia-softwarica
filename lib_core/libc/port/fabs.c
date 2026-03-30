/*s: libc/port/fabs.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[fabs]] */
double
fabs(double arg)
{

    if(arg < 0)
        return -arg;
    return arg;
}
/*e: function [[fabs]] */
/*e: libc/port/fabs.c */
