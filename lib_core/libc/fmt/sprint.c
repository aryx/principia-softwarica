/*s: libc/fmt/sprint.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[sprint]] */
int
sprint(char *buf, char *fmt, ...)
{
    int n;
    va_list args;

    va_start(args, fmt);
    n = vsnprint(buf, 65536, fmt, args);    /* big number, but sprint is deprecated anyway */
    va_end(args);
    return n;
}
/*e: function [[sprint]] */
/*e: libc/fmt/sprint.c */
