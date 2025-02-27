/*s: libc/port/runestrncat.c */
#include <u.h>
#include <libc.h>

/*s: function [[runestrncat]] */
Rune*
runestrncat(Rune *s1, Rune *s2, long n)
{
    Rune *os1;

    os1 = s1;
    s1 = runestrchr(s1, 0);
    while(*s1++ = *s2++)
        if(--n < 0) {
            s1[-1] = 0;
            break;
        }
    return os1;
}
/*e: function [[runestrncat]] */
/*e: libc/port/runestrncat.c */
