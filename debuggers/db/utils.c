/*s: db/utils.c */
#include "defs.h"
#include "fns.h"

/*s: function error */
/*
 * An error occurred; save the message for later printing,
 * close open files, and reset to main command loop.
 */
void
error(char *n)
{
    errmsg = n;
    iclose(0, 1);
    oclose();
    flush();
    delbp();
    ending = 0;
    longjmp(env, 1);
}
/*e: function error */

/*s: function errors */
void
errors(char *m, char *n)
{
    static char buf[128];

    sprint(buf, "%s: %s", m, n);
    error(buf);
}
/*e: function errors */
/*e: db/utils.c */
