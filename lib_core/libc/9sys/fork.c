/*s: libc/9sys/fork.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[fork]] */
int
fork(void)
{
    return rfork(RFPROC|RFFDG|RFREND);
}
/*e: function [[fork]] */
/*e: libc/9sys/fork.c */
