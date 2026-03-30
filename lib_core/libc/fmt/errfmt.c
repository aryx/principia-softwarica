/*s: libc/fmt/errfmt.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
#include "fmtdef.h"

/*s: function [[errfmt]] */
int
errfmt(Fmt *f)
{
    char buf[ERRMAX];

    rerrstr(buf, sizeof buf);
    return _fmtcpy(f, buf, utflen(buf), strlen(buf));
}
/*e: function [[errfmt]] */
/*e: libc/fmt/errfmt.c */
