/*s: libc/port/runestrlen.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[runestrlen]] */
long
runestrlen(Rune *s)
{

    return runestrchr(s, 0) - s;
}
/*e: function [[runestrlen]] */
/*e: libc/port/runestrlen.c */
