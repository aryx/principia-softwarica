/*s: lib_graphics/libdraw/rectclip.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function rectclip */
bool
rectclip(Rectangle *rp, Rectangle b)		/* first by reference, second by value */
{
    Rectangle *bp = &b;
    /*
     * Expand rectXrect() in line for speed
     */
    if((rp->min.x < bp->max.x && bp->min.x < rp->max.x &&
        rp->min.y < bp->max.y && bp->min.y < rp->max.y)==false)
        return false;

    /* They must overlap */
    if(rp->min.x < bp->min.x)
        rp->min.x = bp->min.x;
    if(rp->min.y < bp->min.y)
        rp->min.y = bp->min.y;
    if(rp->max.x > bp->max.x)
        rp->max.x = bp->max.x;
    if(rp->max.y > bp->max.y)
        rp->max.y = bp->max.y;
    return true;
}
/*e: function rectclip */
/*e: lib_graphics/libdraw/rectclip.c */
