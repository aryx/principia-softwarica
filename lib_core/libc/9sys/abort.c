/*s: libc/9sys/abort.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[abort]] */
void
abort(void)
{
    while(*(int*)0)
        ;
}
/*e: function [[abort]] */
/*e: libc/9sys/abort.c */
