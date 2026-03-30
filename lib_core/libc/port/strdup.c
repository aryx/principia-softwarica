/*s: libc/port/strdup.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[strdup]] */
char*
strdup(char *s)
{
    char *ns;

    ns = malloc(strlen(s) + 1);
    if(ns == nil)
        return nil;
    setmalloctag(ns, getcallerpc(&s));

    return strcpy(ns, s);
}
/*e: function [[strdup]] */
/*e: libc/port/strdup.c */
