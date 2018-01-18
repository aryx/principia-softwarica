/*s: libc/port/runestrcmp.c */
#include <u.h>
#include <libc.h>

/*s: function [[runestrcmp]] */
int
runestrcmp(Rune *s1, Rune *s2)
{
    Rune c1, c2;

    for(;;) {
        c1 = *s1++;
        c2 = *s2++;
        if(c1 != c2) {
            if(c1 > c2)
                return 1;
            return -1;
        }
        if(c1 == 0)
            return 0;
    }
}
/*e: function [[runestrcmp]] */
/*e: libc/port/runestrcmp.c */
