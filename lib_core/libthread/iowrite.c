/*s: libthread/iowrite.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

/*s: function [[_iowrite]] */
static long
_iowrite(va_list *arg)
{
    int fd;
    void *a;
    long n;

    fd = va_arg(*arg, int);
    a = va_arg(*arg, void*);
    n = va_arg(*arg, long);
    return write(fd, a, n);
}
/*e: function [[_iowrite]] */

/*s: function [[iowrite]] */
long
iowrite(Ioproc *io, int fd, void *a, long n)
{
    return iocall(io, _iowrite, fd, a, n);
}
/*e: function [[iowrite]] */
/*e: libthread/iowrite.c */
