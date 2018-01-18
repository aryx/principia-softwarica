/*s: port/runestrncmp.c */
#include <u.h>
#include <libc.h>

/*s: function [[runestrncmp]] */
int
runestrncmp(Rune *s1, Rune *s2, long n)
{
    Rune c1, c2;

    while(n > 0) {
        c1 = *s1++;
        c2 = *s2++;
        n--;
        if(c1 != c2) {
            if(c1 > c2)
                return 1;
            return -1;
        }
        if(c1 == 0)
            break;
    }
    return 0;
}
/*e: function [[runestrncmp]] */
/*e: port/runestrncmp.c */
