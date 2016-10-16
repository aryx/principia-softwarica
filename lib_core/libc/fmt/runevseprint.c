/*s: fmt/runevseprint.c */
#include <u.h>
#include <libc.h>

/*s: function runevseprint */
Rune*
runevseprint(Rune *buf, Rune *e, char *fmt, va_list args)
{
    Fmt f;

    if(e <= buf)
        return nil;
    f.runes = 1;
    f.start = buf;
    f.to = buf;
    f.stop = e - 1;
    f.flush = nil;
    f.farg = nil;
    f.nfmt = 0;
    f.args = args;
    dofmt(&f, fmt);
    *(Rune*)f.to = '\0';
    return f.to;
}
/*e: function runevseprint */

/*e: fmt/runevseprint.c */
