/*s: port/strcpy.c */
#include <u.h>
#include <libc.h>
/*s: constant [[N]] */
#define N   10000
/*e: constant [[N]] */

/*s: function [[strcpy]] */
char*
strcpy(char *s1, char *s2)
{
    char *os1;

    os1 = s1;
    while(!memccpy(s1, s2, 0, N)) {
        s1 += N;
        s2 += N;
    }
    return os1;
}
/*e: function [[strcpy]] */
/*e: port/strcpy.c */
