/*s: libc/fmt/runeseprint.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[runeseprint]] */
Rune*
runeseprint(Rune *buf, Rune *e, char *fmt, ...)
{
    Rune *p;
    va_list args;

    va_start(args, fmt);
    p = runevseprint(buf, e, fmt, args);
    va_end(args);
    return p;
}
/*e: function [[runeseprint]] */
/*e: libc/fmt/runeseprint.c */
