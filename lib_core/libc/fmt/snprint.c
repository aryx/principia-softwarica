/*s: fmt/snprint.c */
#include <u.h>
#include <libc.h>

/*s: function snprint */
int
snprint(char *buf, int len, char *fmt, ...)
{
    int n;
    va_list args;

    va_start(args, fmt);
    n = vsnprint(buf, len, fmt, args);
    va_end(args);
    return n;
}
/*e: function snprint */

/*e: fmt/snprint.c */
