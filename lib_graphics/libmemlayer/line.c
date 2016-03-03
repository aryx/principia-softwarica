/*s: lib_graphics/libmemlayer/line.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <memlayer.h>

/*s: struct Lline */
struct Lline
{
    Point			p0;
    Point			p1;
    Point			delta;
    int			end0;
    int			end1;
    int			radius;
    Point			sp;

    Memlayer		*dstlayer;
    Memimage	*src;
    int			op;
};
/*e: struct Lline */

static void llineop(Memimage*, Rectangle, Rectangle, void*, int);

/*s: function _memline */
static
void
_memline(Memimage *dst, Point p0, Point p1, int end0, int end1, int radius, Memimage *src, Point sp, Rectangle clipr, int op)
{
    Memlayer *dl;
    /*s: [[_memline()]] other locals */
    bool srcclipped = false;
    Point d;
    /*x: [[_memline()]] other locals */
    Rectangle r;
    /*x: [[_memline()]] other locals */
    struct Lline ll;
    /*e: [[_memline()]] other locals */

    /*s: [[_memline()]] sanity check radius */
    if(radius < 0)
        return;
    /*e: [[_memline()]] sanity check radius */
    /*s: [[_memline()]] sanity check no src layer */
    if(src->layer)	/* can't draw line with layered source */
        return;
    /*e: [[_memline()]] sanity check no src layer */

    Top:
    dl = dst->layer;
    if(dl == nil){
        // back to memdraw/
        _memimageline(dst, p0, p1, end0, end1, radius, src, sp, clipr, op);
        return;
    }
    /*s: [[_memline()]] when have layers */
    /*s: [[_memline()]] clip source */
    if(!srcclipped){
        d = subpt(sp, p0);
        if(!rectclip(&clipr, rectsubpt(src->clipr, d)))
            return;
        if(!(src->flags&Frepl) && !rectclip(&clipr, rectsubpt(src->r, d)))
            return;
        srcclipped = true;
    }
    /*e: [[_memline()]] clip source */

    // Convert to screen coordinates
    /*s: [[_memline()]] convert p0, p1, clipr coordinates  */
    /* dst is known to be a layer */
    // p0 = addpt(p0, dl->delta)
    p0.x += dl->delta.x;
    p0.y += dl->delta.y;
    // p1 = addpt(p1, dl->delta)
    p1.x += dl->delta.x;
    p1.y += dl->delta.y;
    // clipr = rectaddpt(clipr, dl->elta)
    clipr.min.x += dl->delta.x;
    clipr.min.y += dl->delta.y;
    clipr.max.x += dl->delta.x;
    clipr.max.y += dl->delta.y;
    /*e: [[_memline()]] convert p0, p1, clipr coordinates  */

    /*s: [[_memline()]] if dst is fully visible can optimize */
    if(dl->clear){
        dst = dst->layer->screen->image;
        goto Top;
    }
    /*e: [[_memline()]] if dst is fully visible can optimize */

    /*s: [[_memline()]] set r bounding box and clip it */
    /* can't use sutherland-cohen clipping because lines are wide */
    r = memlinebbox(p0, p1, end0, end1, radius);
    /*
     * r is now a bounding box for the line;
     * use it as a clipping rectangle for subdivision
     */
    if(!rectclip(&r, clipr))
        return;
    /*e: [[_memline()]] set r bounding box and clip it */

    /*s: [[_memline()]] general case, call _memlayerop */
    ll.p0 = p0;
    ll.p1 = p1;
    ll.end0 = end0;
    ll.end1 = end1;
    ll.sp = sp;
    ll.dstlayer = dst->layer;
    ll.src = src;
    ll.radius = radius;
    ll.delta = dl->delta;
    ll.op = op;

    _memlayerop(llineop, dst, r, r, &ll);
    /*e: [[_memline()]] general case, call _memlayerop */
    /*e: [[_memline()]] when have layers */
}
/*e: function _memline */

/*s: function llineop */
static
void
llineop(Memimage *dst, Rectangle screenr, Rectangle clipr, void *etc, bool insave)
{
    struct Lline *ll = etc;
    Point p0, p1;

    USED(screenr.min.x);

    if(insave && ll->dstlayer->save == nil)
        return;
    if(!rectclip(&clipr, screenr))
        return;

    if(insave){
        p0 = subpt(ll->p0, ll->delta);
        p1 = subpt(ll->p1, ll->delta);
        clipr = rectsubpt(clipr, ll->delta);
    }else{
        p0 = ll->p0;
        p1 = ll->p1;
    }

    _memline(dst, p0, p1, ll->end0, ll->end1, ll->radius, ll->src, ll->sp, clipr, ll->op);
}
/*e: function llineop */

/*s: function memline */
void
memline(Memimage *dst, Point p0, Point p1, int end0, int end1, int radius, Memimage *src, Point sp, int op)
{
    _memline(dst, p0, p1, end0, end1, radius, src, sp, dst->clipr, op);
}
/*e: function memline */
/*e: lib_graphics/libmemlayer/line.c */
