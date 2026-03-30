/*s: libc/port/strlen.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[strlen]] */
long
strlen(char *s)
{

    return strchr(s, '\0') - s;
}
/*e: function [[strlen]] */
/*e: libc/port/strlen.c */
