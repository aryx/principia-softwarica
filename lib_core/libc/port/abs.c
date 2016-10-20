/*s: port/abs.c */
#include <u.h>
#include <libc.h>

/*s: function abs */
int
abs(int a)
{
    if(a < 0)
        return -a;
    return a;
}
/*e: function abs */

/*e: port/abs.c */
