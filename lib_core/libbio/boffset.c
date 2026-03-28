/*s: libbio/boffset.c */
/*s: libbio includes */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>
/*e: libbio includes */
/*s: function [[Boffset]] */
vlong
Boffset(Biobufhdr *bp)
{
    vlong n;

    switch(bp->state) {
    default:
        fprint(2, "Boffset: unknown state %d\n", bp->state);
        n = Beof;
        break;

    case Bracteof:
    case Bractive:
        n = bp->offset + bp->icount;
        break;

    case Bwactive:
        n = bp->offset + (bp->bsize + bp->ocount);
        break;
    }
    return n;
}
/*e: function [[Boffset]] */
/*e: libbio/boffset.c */
