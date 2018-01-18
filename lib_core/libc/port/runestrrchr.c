/*s: port/runestrrchr.c */
#include <u.h>
#include <libc.h>

/*s: function [[runestrrchr]] */
Rune*
runestrrchr(Rune *s, Rune c)
{
    Rune *r;

    if(c == 0)
        return runestrchr(s, 0);
    r = 0;
    while(s = runestrchr(s, c))
        r = s++;
    return r;
}
/*e: function [[runestrrchr]] */
/*e: port/runestrrchr.c */
