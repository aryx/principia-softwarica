/*s: fmt/fmtlock.c */
#include <u.h>
#include <libc.h>

/*s: global [[fmtl]] */
static Lock fmtl;
/*e: global [[fmtl]] */

/*s: function [[_fmtlock]] */
void
_fmtlock(void)
{
    lock(&fmtl);
}
/*e: function [[_fmtlock]] */

/*s: function [[_fmtunlock]] */
void
_fmtunlock(void)
{
    unlock(&fmtl);
}
/*e: function [[_fmtunlock]] */
/*e: fmt/fmtlock.c */
