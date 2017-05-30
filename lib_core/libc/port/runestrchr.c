/*s: port/runestrchr.c */
#include <u.h>
#include <libc.h>

/*s: function runestrchr */
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
/*e: function runestrchr */
/*e: port/runestrchr.c */
