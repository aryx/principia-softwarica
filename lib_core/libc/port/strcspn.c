/*s: port/strcspn.c */
#include <u.h>
#include <libc.h>

/*s: constant N (port/strcspn.c) */
#define N   256
/*e: constant N (port/strcspn.c) */

/*s: function [[strcspn]] */
long
strcspn(char *s, char *b)
{
    char map[N], *os;

    memset(map, 0, N);
    for(;;) {
        map[*(uchar*)b] = 1;
        if(*b++ == 0)
            break;
    }
    os = s;
    while(map[*(uchar*)s++] == 0)
        ;
    return s - os - 1;
}
/*e: function [[strcspn]] */
/*e: port/strcspn.c */
