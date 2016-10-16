/*s: fmt/vfprint.c */
#include <u.h>
#include <libc.h>
#include "fmtdef.h"

/*s: function _fmtFdFlush */
/*
 * generic routine for flushing a formatting buffer
 * to a file descriptor
 */
int
_fmtFdFlush(Fmt *f)
{
    int n;

    n = (char*)f->to - (char*)f->start;
    if(n && write((int)(uintptr)f->farg, f->start, n) != n)
        return 0;
    f->to = f->start;
    return 1;
}
/*e: function _fmtFdFlush */

/*s: function vfprint */
int
vfprint(int fd, char *fmt, va_list args)
{
    Fmt f;
    char buf[256];
    int n;

    fmtfdinit(&f, fd, buf, sizeof(buf));
    f.args = args;
    n = dofmt(&f, fmt);
    if(n > 0 && _fmtFdFlush(&f) == 0)
        return -1;
    return n;
}
/*e: function vfprint */
/*e: fmt/vfprint.c */
