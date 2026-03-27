/*s: libbio/bfildes.c */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>

/*s: function [[Bfildes]] */
int
Bfildes(Biobufhdr *bp)
{

    return bp->fid;
}
/*e: function [[Bfildes]] */
/*e: libbio/bfildes.c */
