/*s: port/strchr.c */
#include <u.h>
#include <libc.h>

/*s: function strchr */
char*
strchr(char *s, int c)
{
    char c0 = c;
    char c1;

    if(c == 0) {
        while(*s++)
            ;
        return s-1;
    }

    while(c1 = *s++)
        if(c1 == c0)
            return s-1;
    return 0;
}
/*e: function strchr */
/*e: port/strchr.c */
