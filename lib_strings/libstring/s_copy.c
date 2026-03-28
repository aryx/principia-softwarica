/*s: libstring/s_copy.c */
/*s: libstring includes */
#include <u.h>
#include <libc.h>
#include <str.h> // was string.h
/*e: libstring includes */

/*s: function [[s_copy]] */
/* return a String containing a copy of the passed char array */
extern String*
s_copy(char *cp)
{
    String *sp;
    int len;

    len = strlen(cp)+1;
    sp = s_newalloc(len);
    setmalloctag(sp, getcallerpc(&cp));
    strcpy(sp->base, cp);
    sp->ptr = sp->base + len - 1;		/* point to 0 terminator */
    return sp;
}
/*e: function [[s_copy]] */
/*e: libstring/s_copy.c */
