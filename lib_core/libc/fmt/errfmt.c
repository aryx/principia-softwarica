/*s: fmt/errfmt.c */
#include <u.h>
#include <libc.h>
#include "fmtdef.h"

/*s: function errfmt */
int
errfmt(Fmt *f)
{
    char buf[ERRMAX];

    rerrstr(buf, sizeof buf);
    return _fmtcpy(f, buf, utflen(buf), strlen(buf));
}
/*e: function errfmt */
/*e: fmt/errfmt.c */
