/*s: lib_graphics/libdraw/replclipr.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

/*s: function replclipr */
void
replclipr(Image *i, bool repl, Rectangle clipr)
{
    /*s: [[replclipr()]] body */
    byte *b;

    // set repl and clip: 'c' dstid[4] repl[1] clipR[4*4]
    b = bufimage(i->display, 1+4+1+4*4);
    b[0] = 'c';
    BPLONG(b+1, i->id);
    repl = (repl != 0);
    b[5] = repl;
    BPLONG(b+6, clipr.min.x);
    BPLONG(b+10, clipr.min.y);
    BPLONG(b+14, clipr.max.x);
    BPLONG(b+18, clipr.max.y);
    i->repl = repl;
    i->clipr = clipr;
    /*e: [[replclipr()]] body */
}
/*e: function replclipr */

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
