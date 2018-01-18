/*s: port/strecpy.c */
#include <u.h>
#include <libc.h>

/*s: function [[strecpy]] */
char*
strecpy(char *to, char *e, char *from)
{
    if(to >= e)
        return to;
    to = memccpy(to, from, '\0', e - to);
    if(to == nil){
        to = e - 1;
        *to = '\0';
    }else{
        to--;
    }
    return to;
}
/*e: function [[strecpy]] */
/*e: port/strecpy.c */
