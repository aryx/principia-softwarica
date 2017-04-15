/*s: lib_graphics/libdraw/replclipr.c */
#include <u.h>
#include <libc.h>
#include <draw.h>


/*s: function rectclip */
bool
rectclip(Rectangle *rp, Rectangle b) /* first by reference, second by value */
{
    Rectangle *bp = &b;
    /*
     * Expand rectXrect() in line for speed
     */
    if(!(rp->min.x < bp->max.x && bp->min.x < rp->max.x &&
        rp->min.y < bp->max.y && bp->min.y < rp->max.y))
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

/*s: function drawreplxy */
int
drawreplxy(int min, int max, int x)
{
    int sx;

    sx = (x-min) % (max-min);
    if(sx < 0)
        sx += max - min;
    return sx + min;
}
/*e: function drawreplxy */

/*s: function drawrepl */
Point
drawrepl(Rectangle r, Point p)
{
    p.x = drawreplxy(r.min.x, r.max.x, p.x);
    p.y = drawreplxy(r.min.y, r.max.y, p.y);
    return p;
}
/*e: function drawrepl */

/*e: lib_graphics/libdraw/replclipr.c */
