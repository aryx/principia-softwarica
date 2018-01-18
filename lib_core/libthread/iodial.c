/*s: lib_core/libthread/iodial.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

/*s: function [[_iodial]] */
static long
_iodial(va_list *arg)
{
    char *addr, *local, *dir;
    int *cdfp;

    addr = va_arg(*arg, char*);
    local = va_arg(*arg, char*);
    dir = va_arg(*arg, char*);
    cdfp = va_arg(*arg, int*);

    return dial(addr, local, dir, cdfp);
}
/*e: function [[_iodial]] */

/*s: function [[iodial]] */
int
iodial(Ioproc *io, char *addr, char *local, char *dir, int *cdfp)
{
    return iocall(io, _iodial, addr, local, dir, cdfp);
}
/*e: function [[iodial]] */
/*e: lib_core/libthread/iodial.c */
