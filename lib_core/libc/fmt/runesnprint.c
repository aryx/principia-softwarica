/*s: libc/fmt/runesnprint.c */
#include <u.h>
#include <libc.h>

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
