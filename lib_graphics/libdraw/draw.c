/*s: lib_graphics/libdraw/draw.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

/*s: function [[_setdrawop]] */
void
_setdrawop(Display *d, Drawop op)
{
    uchar *a;

    if(op != SoverD){
        a = bufimage(d, 1+1);
        /*s: [[_setdrawop()]] sanity check a */
        if(a == nil)
            return; // warning?
        /*e: [[_setdrawop()]] sanity check a */
        a[0] = 'O';
        a[1] = op;
    }
}
/*e: function [[_setdrawop]] */
        
/*s: function [[draw1]] */
static void
draw1(Image *dst, Rectangle *r, Image *src, Point *p0, Image *mask, Point *p1, Drawop op)
{
    byte *a;

    _setdrawop(dst->display, op);

    // draw: 'd' dstid[4] srcid[4] maskid[4] R[4*4] P[2*4] P[2*4]
    a = bufimage(dst->display, 1+4+4+4+4*4+2*4+2*4);
    /*s: [[draw1()]] sanity check a */
    if(a == nil)
        return; // warning?
    /*e: [[draw1()]] sanity check a */
    /*s: [[draw1()]] sanity check src and mask */
    if(src == nil)
        src = dst->display->black;
    if(mask == nil)
        mask = dst->display->opaque;
    /*e: [[draw1()]] sanity check src and mask */
    a[0] = 'd';
    BPLONG(a+1, dst->id);
    BPLONG(a+5, src->id);
    BPLONG(a+9, mask->id);
    BPLONG(a+13, r->min.x);
    BPLONG(a+17, r->min.y);
    BPLONG(a+21, r->max.x);
    BPLONG(a+25, r->max.y);
    BPLONG(a+29, p0->x);
    BPLONG(a+33, p0->y);
    BPLONG(a+37, p1->x);
    BPLONG(a+41, p1->y);
}
/*e: function [[draw1]] */

/*s: function [[draw]] */
void
draw(Image *dst, Rectangle r, Image *src, Image *mask, Point p1)
{
    draw1(dst, &r, src, &p1, mask, &p1, SoverD);
}
/*e: function [[draw]] */

/*s: function [[drawop]] */
void
drawop(Image *dst, Rectangle r, Image *src, Image *mask, Point p1, Drawop op)
{
    draw1(dst, &r, src, &p1, mask, &p1, op);
}
/*e: function [[drawop]] */

/*s: function [[gendraw]] */
void
gendraw(Image *dst, Rectangle r, Image *src, Point p0, Image *mask, Point p1)
{
    draw1(dst, &r, src, &p0, mask, &p1, SoverD);
}
/*e: function [[gendraw]] */

/*s: function [[gendrawop]] */
void
gendrawop(Image *dst, Rectangle r, Image *src, Point p0, Image *mask, Point p1, Drawop op)
{
    draw1(dst, &r, src, &p0, mask, &p1, op);
}
/*e: function [[gendrawop]] */

/*s: function [[border]] */
void
border(Image *im, Rectangle r, int i, Image *color, Point sp)
{
    /*s: [[border()]] if negative i, border goes outside */
    if(i < 0){
        r = insetrect(r, i);
        sp = addpt(sp, Pt(i,i));
        i = -i;
    }
    /*e: [[border()]] if negative i, border goes outside */
    // horizontal bars
    draw(im, Rect(r.min.x, r.min.y,   r.max.x, r.min.y+i),
        color, nil, sp);
    draw(im, Rect(r.min.x, r.max.y-i,   r.max.x, r.max.y),
        color, nil, Pt(sp.x, sp.y+Dy(r)-i));
    // vertical bars
    draw(im, Rect(r.min.x, r.min.y+i,   r.min.x+i, r.max.y-i),
        color, nil, Pt(sp.x, sp.y+i));
    draw(im, Rect(r.max.x-i, r.min.y+i,   r.max.x, r.max.y-i),
        color, nil, Pt(sp.x+Dx(r)-i, sp.y+i));
}
/*e: function [[border]] */
/*e: lib_graphics/libdraw/draw.c */
