/*s: port/strpbrk.c */
#include <u.h>
#include <libc.h>
/*s: constant N (port/strpbrk.c) */
#define N   256
/*e: constant N (port/strpbrk.c) */

/*s: function strpbrk */
char*
strpbrk(char *cs, char *cb)
{
    char map[N];
    uchar *s=(uchar*)cs, *b=(uchar*)cb;

    memset(map, 0, N);
    for(;;) {
        map[*b] = 1;
        if(*b++ == 0)
            break;
    }
    while(map[*s++] == 0)
        ;
    if(*--s)
        return (char*)s;
    return 0;
}
/*e: function strpbrk */
/*e: port/strpbrk.c */
