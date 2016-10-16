/*s: port/strcmp.c */
#include <u.h>
#include <libc.h>

/*s: function strcmp */
int
strcmp(char *s1, char *s2)
{
    unsigned c1, c2;

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
/*e: function strcmp */
/*e: port/strcmp.c */
