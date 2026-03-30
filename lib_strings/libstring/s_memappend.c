/*s: libstring/s_memappend.c */
/*s: libstring includes */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
#include <str.h> // was string.h
/*e: libstring includes */
/*s: function [[s_memappend]] */
/* append a char array ( of up to n characters) to a String */
String *
s_memappend(String *to, char *from, int n)
{
    char *e;

    if (to == 0)
        to = s_new();
    if (from == 0)
        return to;
    for(e = from + n; from < e; from++)
        s_putc(to, *from);
    s_terminate(to);
    return to;
}
/*e: function [[s_memappend]] */
/*e: libstring/s_memappend.c */
