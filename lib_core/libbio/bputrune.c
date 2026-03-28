/*s: libbio/bputrune.c */
/*s: libbio includes */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>
/*e: libbio includes */
/*s: function [[Bputrune]] */
int
Bputrune(Biobufhdr *bp, long c)
{
    Rune rune;
    char str[UTFmax];
    int n;

    rune = c;
    if(rune < Runeself) {
        Bputc(bp, rune);
        return 1;
    }
    n = runetochar(str, &rune);
    if(n == 0)
        return Bbad;
    if(Bwrite(bp, str, n) != n)
        return Beof;
    return n;
}
/*e: function [[Bputrune]] */
/*e: libbio/bputrune.c */
