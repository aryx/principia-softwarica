/*s: libstring/s_reset.c */
/*s: libstring includes */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
#include <str.h> // was string.h
/*e: libstring includes */

/*s: function [[s_reset]] */
String*
s_reset(String *s)
{
    if(s != nil){
        s = s_unique(s);
        s->ptr = s->base;
        *s->ptr = '\0';
    } else
        s = s_new();
    return s;
}
/*e: function [[s_reset]] */
/*s: function [[s_restart]] */
String*
s_restart(String *s)
{
    s = s_unique(s);
    s->ptr = s->base;
    return s;
}
/*e: function [[s_restart]] */
/*e: libstring/s_reset.c */
