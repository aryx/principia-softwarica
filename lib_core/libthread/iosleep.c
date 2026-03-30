/*s: libthread/iosleep.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
#include <thread.h>
#include "threadimpl.h"

/*s: function [[_iosleep]] */
static long
_iosleep(va_list *arg)
{
    long n;

    n = va_arg(*arg, long);
    return sleep(n);
}
/*e: function [[_iosleep]] */

/*s: function [[iosleep]] */
int
iosleep(Ioproc *io, long n)
{
    return iocall(io, _iosleep, n);
}
/*e: function [[iosleep]] */
/*e: libthread/iosleep.c */
