/*s: 9sys/abort.c */
#include <u.h>
#include <libc.h>
/*s: function abort */
void
abort(void)
{
    while(*(int*)0)
        ;
}
/*e: function abort */
/*e: 9sys/abort.c */
