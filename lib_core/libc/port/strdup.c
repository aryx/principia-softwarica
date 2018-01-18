/*s: port/strdup.c */
#include <u.h>
#include <libc.h>

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
/*e: port/strdup.c */
