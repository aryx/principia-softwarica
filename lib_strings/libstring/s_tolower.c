/*s: libstring/s_tolower.c */
/*s: libstring includes */
#include <u.h>
#include <libc.h>
#include <str.h> // was string.h
/*e: libstring includes */


/*s: function [[s_tolower]] */
/* convert String to lower case */
void
s_tolower(String *sp)
{
    char *cp;

    for(cp=sp->ptr; *cp; cp++)
        *cp = tolower(*cp);
}
/*e: function [[s_tolower]] */
/*e: libstring/s_tolower.c */
