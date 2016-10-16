/*s: port/strspn.c */
#include <u.h>
#include <libc.h>

/*s: constant N (port/strspn.c) */
#define N   256
/*e: constant N (port/strspn.c) */

/*s: function strspn */
long
strspn(char *s, char *b)
{
    char map[N], *os;

    memset(map, 0, N);
    while(*b)
        map[*(uchar *)b++] = 1;
    os = s;
    while(map[*(uchar *)s++])
        ;
    return s - os - 1;
}
/*e: function strspn */
/*e: port/strspn.c */
