/*s: port/_assert.c */
#include <u.h>
#include <libc.h>

/*s: global [[__assert]] */
void (*__assert)(char*);
/*e: global [[__assert]] */


/*s: function [[default_assert]] */
void
default_assert(char *s)
{
    if(__assert)
        (*__assert)(s);
    fprint(2, "assert failed: %s\n", s);
    abort();
}
/*e: function [[default_assert]] */

/*s: global [[_assert]] */
void (*_assert)(char*) = default_assert;
/*e: global [[_assert]] */
/*e: port/_assert.c */
