/*s: libc/port/runestrdup.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[runestrdup]] */
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
/*e: function [[runestrdup]] */
/*e: libc/port/runestrdup.c */
