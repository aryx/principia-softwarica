/*s: libstring/s_append.c */
/*s: libstring includes */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
#include <str.h> // was string.h
/*e: libstring includes */

/*s: function [[s_append]] */
/* append a char array to a String */
String *
s_append(String *to, char *from)
{
    if (to == 0)
        to = s_new();
    if (from == 0)
        return to;
    for(; *from; from++)
        s_putc(to, *from);
    s_terminate(to);
    return to;
}
/*e: function [[s_append]] */
/*e: libstring/s_append.c */
