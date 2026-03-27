/*s: libbio/bputc.c */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>

/*s: function [[Bputc]] */
int
Bputc(Biobufhdr *bp, int c)
{
    int i;

    for(;;) {
        i = bp->ocount;
        if(i) {
            bp->ebuf[i++] = c;
            bp->ocount = i;
            return 0;
        }
        if(Bflush(bp) == Beof)
            break;
    }
    return Beof;
}
/*e: function [[Bputc]] */
/*e: libbio/bputc.c */
