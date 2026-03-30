/*s: libstring/s_unique.c */
/*s: libstring includes */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
#include <str.h> // was string.h
/*e: libstring includes */

/*s: function [[s_unique]] */
String*
s_unique(String *s)
{
    String *p;

    if(s->ref > 1){
        p = s;
        s = s_clone(p);
        s_free(p);
    }
    return s;
}
/*e: function [[s_unique]] */
/*e: libstring/s_unique.c */
