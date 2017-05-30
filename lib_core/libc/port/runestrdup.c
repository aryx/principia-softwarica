/*s: port/runestrdup.c */
#include <u.h>
#include <libc.h>

/*s: function runestrdup */
Rune*
runestrdup(Rune *s) 
{  
    Rune *ns;

    ns = malloc(sizeof(Rune)*(runestrlen(s) + 1));
    if(ns == nil)
        return nil;
    setmalloctag(ns, getcallerpc(&s));

    return runestrcpy(ns, s);
}
/*e: function runestrdup */
/*e: port/runestrdup.c */
