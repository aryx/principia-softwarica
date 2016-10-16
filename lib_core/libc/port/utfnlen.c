/*s: port/utfnlen.c */
#include <u.h>
#include <libc.h>

/*s: function utfnlen */
int
utfnlen(char *s, long m)
{
    int c;
    long n;
    Rune rune;
    char *es;

    es = s + m;
    for(n = 0; s < es; n++) {
        c = *(uchar*)s;
        if(c < Runeself){
            if(c == '\0')
                break;
            s++;
            continue;
        }
        if(!fullrune(s, es-s))
            break;
        s += chartorune(&rune, s);
    }
    return n;
}
/*e: function utfnlen */
/*e: port/utfnlen.c */
