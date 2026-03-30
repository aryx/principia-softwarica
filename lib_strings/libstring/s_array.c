/*s: libstring/s_array.c */
/*s: libstring includes */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
#include <str.h> // was string.h
/*e: libstring includes */

extern String*	_s_alloc(void);

/*s: function [[s_array]] */
/* return a String containing a character array (this had better not grow) */
extern String *
s_array(char *cp, int len)
{
    String *sp = _s_alloc();

    sp->base = sp->ptr = cp;
    sp->end = sp->base + len;
    sp->fixed = 1;
    return sp;
}
/*e: function [[s_array]] */
/*e: libstring/s_array.c */
