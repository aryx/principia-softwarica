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

/*s: function labs */
long
labs(long a)
{
    if(a < 0)
        return -a;
    return a;
}
/*e: function labs */
/*e: port/abs.c */
