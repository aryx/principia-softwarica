/*s: libstring/s_nappend.c */
/*s: libstring includes */
#include <u.h>
#include <libc.h>
#include <str.h> // was string.h
/*e: libstring includes */

/*s: function [[s_nappend]] */
/* append a char array ( of up to n characters) to a String */
String *
s_nappend(String *to, char *from, int n)
{
    if (to == 0)
        to = s_new();
    if (from == 0)
        return to;
    for(; n && *from; from++, n--)
        s_putc(to, *from);
    s_terminate(to);
    return to;
}
/*e: function [[s_nappend]] */
/*e: libstring/s_nappend.c */
