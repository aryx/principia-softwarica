/*s: mk/utils.c */
#include "mk.h"

/*s: function Malloc */
void *
Malloc(int n)
{
    register void *s;

    s = malloc(n);
    if(!s) {
        fprint(2, "mk: cannot alloc %d bytes\n", n);
        Exit();
    }
    return(s);
}
/*e: function Malloc */

/*s: function Realloc */
void *
Realloc(void *s, int n)
{
    if(s)
        s = realloc(s, n);
    else
        s = malloc(n);
    if(!s) {
        fprint(2, "mk: cannot alloc %d bytes\n", n);
        Exit();
    }
    return(s);
}
/*e: function Realloc */


/*e: mk/utils.c */
