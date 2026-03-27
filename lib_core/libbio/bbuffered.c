/*s: libbio/bbuffered.c */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>

/*s: function [[Bbuffered]] */
int
Bbuffered(Biobufhdr *bp)
{
    switch(bp->state) {
    case Bracteof:
    case Bractive:
        return -bp->icount;

    case Bwactive:
        return bp->bsize + bp->ocount;

    case Binactive:
        return 0;
    }
    fprint(2, "Bbuffered: unknown state %d\n", bp->state);
    return 0;
}
/*e: function [[Bbuffered]] */
/*e: libbio/bbuffered.c */
