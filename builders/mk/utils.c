/*s: mk/utils.c */
#include "mk.h"

/*s: function [[Malloc]] */
void*
Malloc(int n)
{
    void *s;

    s = malloc(n);
    if(!s) {
        fprint(STDERR, "mk: cannot alloc %d bytes\n", n);
        Exit();
    }
    return s;
}
/*e: function [[Malloc]] */

/*s: function [[Realloc]] */
void *
Realloc(void *s, int n)
{
    if(s)
        s = realloc(s, n);
    else
        s = malloc(n);
    if(!s) {
        fprint(STDERR, "mk: cannot alloc %d bytes\n", n);
        Exit();
    }
    return s;
}
/*e: function [[Realloc]] */

// was in plan9.c
/*s: function [[maketmp]] */
char*
maketmp(void)
{
    static char temp[] = "/tmp/mkargXXXXXX";

    mktemp(temp);
    return temp;
}
/*e: function [[maketmp]] */

/*e: mk/utils.c */
