/*s: 9sys/fork.c */
#include <u.h>
#include <libc.h>

/*s: function fork */
int
fork(void)
{
    return rfork(RFPROC|RFFDG|RFREND);
}
/*e: function fork */
/*e: 9sys/fork.c */
