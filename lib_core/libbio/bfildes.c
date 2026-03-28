/*s: libbio/bfildes.c */
/*s: libbio includes */
#include	<u.h>
#include	<libc.h>
#include	<bio.h>
/*e: libbio includes */
/*s: function [[Bfildes]] */
fdt
Bfildes(Biobufhdr *bp)
{

    return bp->fid;
}
/*e: function [[Bfildes]] */
/*e: libbio/bfildes.c */
