/*s: libthread/ioclose.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

/*s: function [[_ioclose]] */
static long
_ioclose(va_list *arg)
{
    int fd;

    fd = va_arg(*arg, int);
    return close(fd);
}
/*e: function [[_ioclose]] */

/*s: function [[ioclose]] */
int
ioclose(Ioproc *io, int fd)
{
    return iocall(io, _ioclose, fd);
}
/*e: function [[ioclose]] */
/*e: libthread/ioclose.c */
