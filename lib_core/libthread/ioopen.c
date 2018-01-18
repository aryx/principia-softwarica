/*s: libthread/ioopen.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

/*s: function [[_ioopen]] */
static long
_ioopen(va_list *arg)
{
    char *path;
    int mode;

    path = va_arg(*arg, char*);
    mode = va_arg(*arg, int);
    return open(path, mode);
}
/*e: function [[_ioopen]] */

/*s: function [[ioopen]] */
int
ioopen(Ioproc *io, char *path, int mode)
{
    return iocall(io, _ioopen, path, mode);
}
/*e: function [[ioopen]] */
/*e: libthread/ioopen.c */
