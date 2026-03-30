/*s: libc/port/runestrcat.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[runestrcat]] */
Rune*
runestrcat(Rune *s1, Rune *s2)
{

    runestrcpy(runestrchr(s1, 0), s2);
    return s1;
}
/*e: function [[runestrcat]] */
/*e: libc/port/runestrcat.c */
