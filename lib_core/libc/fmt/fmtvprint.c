/*s: libc/fmt/fmtvprint.c */
#include <u.h>
#include <libc.h>
#include "fmtdef.h"


/*s: function [[fmtvprint]] */
/*
 * format a string into the output buffer
 * designed for formats which themselves call fmt
 */
int
fmtvprint(Fmt *f, char *fmt, va_list args)
{
    va_list va;
    int n;

    va = f->args;
    f->args = args;
    n = dofmt(f, fmt);
    f->args = va;
    if(n >= 0)
        return 0;
    return n;
}
/*e: function [[fmtvprint]] */

/*e: libc/fmt/fmtvprint.c */
