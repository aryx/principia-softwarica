/*s: port/strlen.c */
#include <u.h>
#include <libc.h>

/*s: function strlen */
long
strlen(char *s)
{

    return strchr(s, 0) - s;
}
/*e: function strlen */
/*e: port/strlen.c */
