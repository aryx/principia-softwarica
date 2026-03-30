/*s: libc/port/strrchr.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[strrchr]] */
char*
strrchr(char *s, int c)
{
    char *r;

    if(c == '\0')
        return strchr(s, '\0');

    r = 0;
    while(s = strchr(s, c))
        r = s++;
    return r;
}
/*e: function [[strrchr]] */
/*e: libc/port/strrchr.c */
