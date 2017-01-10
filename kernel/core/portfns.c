/*s: portfns.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

// backward dependencies breaker for non functional properties functions
// (logging, security, error, profiling/timing)
/*s: portfns.c backward deps breaker */
void (*coherence)(void) = nil;
int (*iprint)(char*, ...) = nil;
/*e: portfns.c backward deps breaker */

/*s: function returnfalse */
// usually used as default callback for sleep/tsleep
bool
returnfalse(void*)
{
    return false;
}
/*e: function returnfalse */

// was in devcons.c, could be in lib/misc.c
/*s: function readnum */
int
readnum(ulong off, char *buf, ulong n, ulong val, int size)
{
    char tmp[64];

    snprint(tmp, sizeof(tmp), "%*lud", size-1, val);
    tmp[size-1] = ' ';
    if(off >= size)
        return 0;
    if(off+n > size)
        n = size-off;
    memmove(buf, tmp+off, n);
    return n;
}
/*e: function readnum */

/*s: function readstr */
int
readstr(ulong off, char *buf, ulong n, char *str)
{
    int size;

    size = strlen(str);
    if(off >= size)
        return 0;
    if(off+n > size)
        n = size-off;
    memmove(buf, str+off, n);
    return n;
}
/*e: function readstr */
/*e: portfns.c */
