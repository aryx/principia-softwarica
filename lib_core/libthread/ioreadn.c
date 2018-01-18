/*s: lib_core/libthread/ioreadn.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

/*s: function [[_ioreadn]] */
static long
_ioreadn(va_list *arg)
{
    int fd;
    void *a;
    long n;

    fd = va_arg(*arg, int);
    a = va_arg(*arg, void*);
    n = va_arg(*arg, long);
    return readn(fd, a, n);
}
/*e: function [[_ioreadn]] */

/*s: function [[ioreadn]] */
long
ioreadn(Ioproc *io, int fd, void *a, long n)
{
    return iocall(io, _ioreadn, fd, a, n);
}
/*e: function [[ioreadn]] */
/*e: lib_core/libthread/ioreadn.c */
