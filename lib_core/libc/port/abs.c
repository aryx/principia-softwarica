/*s: libc/port/abs.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[abs]] */
int
abs(int a)
{
    if(a < 0)
        return -a;
    return a;
}
/*e: function [[abs]] */
/*e: libc/port/abs.c */
