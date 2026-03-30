/*s: libc/fmt/runesnprint.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[runesnprint]] */
int
runesnprint(Rune *buf, int len, char *fmt, ...)
{
    int n;
    va_list args;

    va_start(args, fmt);
    n = runevsnprint(buf, len, fmt, args);
    va_end(args);
    return n;
}
/*e: function [[runesnprint]] */
/*e: libc/fmt/runesnprint.c */
