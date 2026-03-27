/*s: libstring/s_array.c */
#include <u.h>
#include <libc.h>
#include <string.h>

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
