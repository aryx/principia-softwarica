/*s: port/strrchr.c */
#include <u.h>
#include <libc.h>

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
/*e: port/strrchr.c */
