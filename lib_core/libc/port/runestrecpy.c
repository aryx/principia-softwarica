/*s: libc/port/runestrecpy.c */
/*s: libc includes */
#include    <u.h>
#include    <libc.h>
/*e: libc includes */
/*s: function [[runestrecpy]] */
Rune*
runestrecpy(Rune *s1, Rune *es1, Rune *s2)
{
    if(s1 >= es1)
        return s1;

    while(*s1++ = *s2++){
        if(s1 == es1){
            *--s1 = '\0';
            break;
        }
    }
    return s1;
}
/*e: function [[runestrecpy]] */
/*e: libc/port/runestrecpy.c */
