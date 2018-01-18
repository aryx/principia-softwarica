/*s: libc/port/runestrlen.c */
#include <u.h>
#include <libc.h>

/*s: function [[runestrlen]] */
long
runestrlen(Rune *s)
{

    return runestrchr(s, 0) - s;
}
/*e: function [[runestrlen]] */
/*e: libc/port/runestrlen.c */
