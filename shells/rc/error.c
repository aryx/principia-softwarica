/*s: rc/error.c */
/*s: includes */
#include "rc.h"
#include "getflags.h"
#include "exec.h"
#include "io.h"
#include "fns.h"
/*e: includes */

// was in rc.h
/*s: global [[nerror]] */
int nerror;		/* number of errors encountered during compilation */
/*e: global [[nerror]] */
/*s: global [[err]] */
io *err;
/*e: global [[err]] */

/*s: function [[panic]] */
void
panic(char *s, int n)
{
    pfmt(err, "rc: ");
    pfmt(err, s, n);
    pchr(err, '\n');
    flush(err);
    Abort();
}
/*e: function [[panic]] */
/*e: rc/error.c */
