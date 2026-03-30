/*s: libc/port/runestrchr.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[runestrchr]] */
Rune*
runestrchr(Rune *s, Rune c)
{
    Rune c0 = c;
    Rune c1;

    if(c == 0) {
        while(*s++)
            ;
        return s-1;
    }

    while(c1 = *s++)
        if(c1 == c0)
            return s-1;
    return nil;
}
/*e: function [[runestrchr]] */
/*e: libc/port/runestrchr.c */
