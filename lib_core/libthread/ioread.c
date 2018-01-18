/*s: libthread/ioread.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

/*s: function [[_ioread]] */
static long
_ioread(va_list *arg)
{
    int fd;
    void *a;
    long n;

    fd = va_arg(*arg, int);
    a = va_arg(*arg, void*);
    n = va_arg(*arg, long);
    return read(fd, a, n);
}
/*e: function [[_ioread]] */

/*s: function [[ioread]] */
long
ioread(Ioproc *io, int fd, void *a, long n)
{
    return iocall(io, _ioread, fd, a, n);
}
/*e: function [[ioread]] */
/*e: libthread/ioread.c */
