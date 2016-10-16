/*s: port/utfecpy.c */
#include <u.h>
#include <libc.h>

/*s: function utfecpy */
char*
utfecpy(char *to, char *e, char *from)
{
    char *end;

    if(to >= e)
        return to;
    end = memccpy(to, from, '\0', e - to);
    if(end == nil){
        end = e;
        while(end>to && (*--end&0xC0)==0x80)
            ;
        *end = '\0';
    }else{
        end--;
    }
    return end;
}
/*e: function utfecpy */
/*e: port/utfecpy.c */
