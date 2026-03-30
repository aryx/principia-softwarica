/*s: libc/fmt/fprint.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[fprint]] */
int
fprint(int fd, char *fmt, ...)
{
    int n;
    va_list args;

    va_start(args, fmt);
    n = vfprint(fd, fmt, args);
    va_end(args);
    return n;
}
/*e: function [[fprint]] */
/*e: libc/fmt/fprint.c */
