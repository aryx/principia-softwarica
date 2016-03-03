/*s: lib_graphics/libmemlayer/draw.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <memlayer.h>

#define DBG1 if(0) print
#define DBG if(0) print

/*s: struct Draw */
struct Draw
{
    Memlayer    *dstlayer;

    Memimage	*src;
    Point	deltas;

    Memimage	*mask;
    Point	deltam;

    // enum<Drawop>
    int	op;
};
/*e: struct Draw */

/*s: function ldrawop */
static
void
ldrawop(Memimage *dst, Rectangle screenr, Rectangle clipr, void *etc, bool insave)
{
    struct Draw *d = etc;
    Point p0, p1;
    Rectangle r;
    /*s: [[ldrawop()]] other locals */
    Rectangle oclipr, srcr, mr;
    bool ok;
    /*e: [[ldrawop()]] other locals */

    /*s: [[ldrawop()]] return if no save in dst */
    if(insave && d->dstlayer->save == nil)
        return;
    /*e: [[ldrawop()]] return if no save in dst */

    p0 = addpt(screenr.min, d->deltas);
    p1 = addpt(screenr.min, d->deltam);

    if(insave){
        r     = rectsubpt(screenr, d->dstlayer->delta);
        clipr = rectsubpt(clipr,   d->dstlayer->delta);
    }else
        r = screenr;

    /* now in logical coordinates */

    /*s: [[ldrawop()]] if r not in clipr */
    /* clipr may have narrowed what we should draw on, so clip if necessary */
    if(!rectinrect(r, clipr)){
        /*s: [[ldrawop()]] change dst clipr */
        oclipr = dst->clipr;
        dst->clipr = clipr;
        /*e: [[ldrawop()]] change dst clipr */
        ok = drawclip(dst, &r, d->src, &p0, d->mask, &p1, &srcr, &mr);
        /*s: [[ldrawop()]] restore dst clipr */
        dst->clipr = oclipr;
        /*e: [[ldrawop()]] restore dst clipr */
        if(!ok)
            return;
    }
    /*e: [[ldrawop()]] if r not in clipr */
    memdraw(dst, r, d->src, p0, d->mask, p1, d->op); // will call memimagedraw
}
/*e: function ldrawop */

/*s: function memdraw */
void
memdraw(Memimage *dst, Rectangle r, Memimage *src, Point p0, Memimage *mask, Point p1, int op)
{
    /*s: [[memdraw()]] locals */
    Memlayer *dl, *sl;
    /*x: [[memdraw()]] locals */
    Rectangle srcr, mr; // set by drawclip
    /*x: [[memdraw()]] locals */
    Rectangle tr;
    /*x: [[memdraw()]] locals */
    struct Draw d;
    /*e: [[memdraw()]] locals */

    DBG("memdraw %p %R %p %P %p %P\n", dst, r, src, p0, mask, p1);
    /*s: [[memdraw()]] sanity check mask */
    if(mask == nil)
        mask = memopaque;
    /*e: [[memdraw()]] sanity check mask */

    /*s: [[memdraw()]] if mask has layer */
    if(mask->layer){
        DBG1("mask->layer != nil\n");
        return;	/* too hard, at least for now */
    }
    /*e: [[memdraw()]] if mask has layer */
    Top:
    if(dst->layer == nil && src->layer == nil){
        memimagedraw(dst, r, src, p0, mask, p1, op); // back to memdraw
        return;
    }
    /*s: [[memdraw()]] when have layers */
    /*s: [[memdraw()]] call drawclip, if empty rectangle return */
    if(!drawclip(dst, &r, src, &p0, mask, &p1,   &srcr, &mr)){
        return;
    }
    /*e: [[memdraw()]] call drawclip, if empty rectangle return */

    /*
     Convert to screen coordinates.
     */
    dl = dst->layer;
    /*s: [[memdraw()]] convert [[r]] if dst is a window */
    if(dl != nil){
        // r = rectaddpt(r, dl->delta)
        r.min.x += dl->delta.x;
        r.min.y += dl->delta.y;
        r.max.x += dl->delta.x;
        r.max.y += dl->delta.y;
    }
    /*e: [[memdraw()]] convert [[r]] if dst is a window */

    /*s: [[memdraw()]] if dst is fully visible can optimize */
    Clearlayer:
    if(dl!=nil && dl->clear){
        if(src == dst){
            // p0 = addpt(p0, dl->delta)
            p0.x += dl->delta.x;
            p0.y += dl->delta.y;

            src = dl->screen->image; // progress
        }
        dst = dl->screen->image; // progress
        goto Top;
    }
    /*e: [[memdraw()]] if dst is fully visible can optimize */
    // else

    sl = src->layer;
    /*s: [[memdraw()]] convert [[p0]] and [[srcr]] if src is a window */
    if(sl != nil){
        // p0 = addpt(p0, sl->delta)
        p0.x += sl->delta.x;
        p0.y += sl->delta.y;
        // srcr = rectaddpt(srcr, sl->delta)
        srcr.min.x += sl->delta.x;
        srcr.min.y += sl->delta.y;
        srcr.max.x += sl->delta.x;
        srcr.max.y += sl->delta.y;
    }
    /*e: [[memdraw()]] convert [[p0]] and [[srcr]] if src is a window */

    /*
     * Now everything is in screen coordinates.
     * mask is an image.  dst and src are images or obscured layers.
     */
    /*s: [[memdraw()]] if dst and src are the same window */
    /*
     * if dst and src are the same layer, just draw in save area and expose.
     */
    if(dl != nil && dst == src){
        /*s: [[memdraw()]] if refresh function for dst */
        if(dl->save == nil)
            return;	/* refresh function makes this case unworkable */
        /*e: [[memdraw()]] if refresh function for dst */
        if(rectXrect(r, srcr)){
            /*s: [[memdraw()]] set tr to union of r and srcr and adapt p1 */
            tr = r;
            if(srcr.min.x < tr.min.x){
                p1.x += tr.min.x - srcr.min.x;
                tr.min.x = srcr.min.x;
            }
            if(srcr.min.y < tr.min.y){
                p1.y += tr.min.x - srcr.min.x;
                tr.min.y = srcr.min.y;
            }
            if(srcr.max.x > tr.max.x)
                tr.max.x = srcr.max.x;
            if(srcr.max.y > tr.max.y)
                tr.max.y = srcr.max.y;
            /*e: [[memdraw()]] set tr to union of r and srcr and adapt p1 */
            memlhide(dst, tr);
        }else{
            memlhide(dst, r);
            memlhide(dst, srcr);
        }
        memdraw(dl->save, rectsubpt(r,    dl->delta), 
                dl->save, subpt(srcr.min, dl->delta),  
                mask, p1, op);
        memlexpose(dst, r);
        return;
    }
    /*e: [[memdraw()]] if dst and src are the same window */
    // else

    /*s: [[memdraw()]] make src an image */
    if(sl){
        /*s: [[memdraw()]] if src is fully visible can optimize */
        if(sl->clear){
            src = sl->screen->image;
            if(dl != nil){
                // r = rectsubpt(r, dl->delta)
                r.min.x -= dl->delta.x;
                r.min.y -= dl->delta.y;
                r.max.x -= dl->delta.x;
                r.max.y -= dl->delta.y;
            }
            goto Top;
        }
        /*e: [[memdraw()]] if src is fully visible can optimize */

        /* relatively rare case; use save area */
        /*s: [[memdraw()]] if refresh function for src */
        if(sl->save == nil)
            return;	/* refresh function makes this case unworkable */
        /*e: [[memdraw()]] if refresh function for src */
        memlhide(src, srcr); // draw the needed pixels in sl->save

        /* convert back to logical coordinates */
        // p0 = subpt(p0, sl->delta)
        p0.x -= sl->delta.x;
        p0.y -= sl->delta.y;
        // srcr = rectsubpt(srcr, sl->delta)
        srcr.min.x -= sl->delta.x;
        srcr.min.y -= sl->delta.y;
        srcr.max.x -= sl->delta.x;
        srcr.max.y -= sl->delta.y;

        src = src->layer->save; // progress
    }
    /*e: [[memdraw()]] make src an image */

    /*
     * src is now an image.  dst may be an image or a clear layer
     */
    if(dl == nil)
        goto Top;
    /*s: [[memdraw()]] after src is an image, if dst is fully visible can optimize */
    if(dl->clear)
        goto Clearlayer;
    /*e: [[memdraw()]] after src is an image, if dst is fully visible can optimize */
    // else

    /*s: [[memdraw()]] general case where dst is an obscured window */
    /*
     * dst is an obscured layer
     */
    d.dstlayer = dl;
    d.src = src;
    d.mask = mask;
    d.op = op;
    d.deltas = subpt(p0, r.min);
    d.deltam = subpt(p1, r.min);

    _memlayerop(ldrawop, dst, r, r, &d);
    /*e: [[memdraw()]] general case where dst is an obscured window */
    /*e: [[memdraw()]] when have layers */
}
/*e: function memdraw */
/*e: lib_graphics/libmemlayer/draw.c */
