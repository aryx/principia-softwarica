/*s: libc/port/runestrcat.c */
#include <u.h>
#include <libc.h>

/*s: function [[runestrcat]] */
Rune*
runestrcat(Rune *s1, Rune *s2)
{

    runestrcpy(runestrchr(s1, 0), s2);
    return s1;
}
/*e: function [[runestrcat]] */
/*e: libc/port/runestrcat.c */
