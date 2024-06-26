/*s: libc/port/memccpy.c */
#include    <u.h>
#include    <libc.h>

/*s: function [[memccpy]] */
void*
memccpy(void *a1, void *a2, int c, ulong n)
{
    uchar *s1, *s2;

    s1 = a1;
    s2 = a2;
    c &= 0xFF;
    while(n > 0) {
        if((*s1++ = *s2++) == c)
            return s1;
        n--;
    }
    return nil;
}
/*e: function [[memccpy]] */
/*e: libc/port/memccpy.c */
