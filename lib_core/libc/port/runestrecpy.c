/*s: port/runestrecpy.c */
#include <u.h>
#include <libc.h>

/*s: function runestrecpy */
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
/*e: function runestrecpy */
/*e: port/runestrecpy.c */
