/*s: libc/fmt/runesmprint.c */
#include <u.h>
#include <libc.h>

/*s: function [[runesmprint]] */
Rune*
runesmprint(char *fmt, ...)
{
    va_list args;
    Rune *p;

    va_start(args, fmt);
    p = runevsmprint(fmt, args);
    va_end(args);
    return p;
}
/*e: function [[runesmprint]] */
/*e: libc/fmt/runesmprint.c */
