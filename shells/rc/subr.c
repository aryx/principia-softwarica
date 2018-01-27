/*s: rc/subr.c */
/*s: includes */
#include "rc.h"
#include "getflags.h"
#include "exec.h"
#include "io.h"
#include "fns.h"
/*e: includes */

/*s: global [[bp]] */
char *bp;
/*e: global [[bp]] */

/*s: function [[iacvt]] */
static void
iacvt(int n)
{
    if(n<0){
        *bp++='-';
        n=-n;	/* doesn't work for n==-inf */
    }
    if(n/10)
        iacvt(n/10);
    *bp++=n%10+'0';
}
/*e: function [[iacvt]] */

/*s: function [[inttoascii]] */
void
inttoascii(char *s, long n)
{
    bp = s;
    iacvt(n);
    *bp='\0';
}
/*e: function [[inttoascii]] */

/*e: rc/subr.c */
