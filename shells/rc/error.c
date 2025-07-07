/*s: rc/error.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

// was in rc.h
/*s: global [[nerror]] */
int nerror;		/* number of errors encountered during compilation */
/*e: global [[nerror]] */
/*s: global [[err]] */
io *err;
/*e: global [[err]] */

// forward decls
void Abort(void);

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

// Exit is back in plan9.c

/*s: function [[Abort]] */
void
Abort(void)
{
    pfmt(err, "aborting\n");
    flush(err);
    Exit("aborting", __LOC__);
}
/*e: function [[Abort]] */

/*e: rc/error.c */
