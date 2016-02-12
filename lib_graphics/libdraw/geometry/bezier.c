/*s: lib_graphics/libdraw/bezier.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: constant PINC */
#define	PINC	32		/* realloc granularity */
/*e: constant PINC */

typedef struct Plist Plist;
/*s: struct Plist */
struct Plist
{
    Point *p;
    int np;			/* -1 if malloc/realloc failed */
};
/*e: struct Plist */

/*s: function appendpt */
static void
appendpt(Plist *l, Point p)
{
    if(l->np == -1)
        return;

    if(l->np == 0)
        l->p = malloc(PINC*sizeof(Point));
    else if(l->np % PINC == 0)
        l->p = realloc(l->p, (l->np+PINC)*sizeof(Point));

    if(l->p == nil){
        l->np = -1;
        return;
    }
    l->p[l->np++] = p;
}
/*e: function appendpt */

/*s: function normsq */
static int
normsq(Point p)
{
    return p.x*p.x+p.y*p.y;
}
/*e: function normsq */

/*s: function psdist */
static int
psdist(Point p, Point a, Point b)
{
    int num, den;

    p = subpt(p, a);
    b = subpt(b, a);
    num = p.x*b.x + p.y*b.y;
    if(num <= 0)
        return normsq(p);
    den = normsq(b);
    if(num >= den)
        return normsq(subpt(b, p));
    return normsq(subpt(divpt(mulpt(b, num), den), p));
}
/*e: function psdist */

/*s: function bpts1 */
/*
 * Convert cubic Bezier curve control points to polyline
 * vertices.  Leaves the last vertex off, so you can continue
 * with another curve.
 */
static void
bpts1(Plist *l, Point p0, Point p1, Point p2, Point p3, int scale)
{
    Point p01, p12, p23, p012, p123, p0123;
    Point tp0, tp1, tp2, tp3;
    tp0=divpt(p0, scale);
    tp1=divpt(p1, scale);
    tp2=divpt(p2, scale);
    tp3=divpt(p3, scale);
    if(psdist(tp1, tp0, tp3)<=1 && psdist(tp2, tp0, tp3)<=1){
        appendpt(l, tp0);
        appendpt(l, tp1);
        appendpt(l, tp2);
    }
    else{
        /*
         * if scale factor is getting too big for comfort,
         * rescale now & concede the rounding error
         */
        if(scale>(1<<12)){
            p0=tp0;
            p1=tp1;
            p2=tp2;
            p3=tp3;
            scale=1;
        }
        p01=addpt(p0, p1);
        p12=addpt(p1, p2);
        p23=addpt(p2, p3);
        p012=addpt(p01, p12);
        p123=addpt(p12, p23);
        p0123=addpt(p012, p123);
        bpts1(l, mulpt(p0, 8), mulpt(p01, 4), mulpt(p012, 2), p0123, scale*8);
        bpts1(l, p0123, mulpt(p123, 2), mulpt(p23, 4), mulpt(p3, 8), scale*8);
    }
}
/*e: function bpts1 */

/*s: function bpts */
static void
bpts(Plist *l, Point p0, Point p1, Point p2, Point p3)
{
    bpts1(l, p0, p1, p2, p3, 1);
}
/*e: function bpts */

/*s: function bezierpts */
static void
bezierpts(Plist *l, Point p0, Point p1, Point p2, Point p3)
{
    bpts(l, p0, p1, p2, p3);
    appendpt(l, p3);
}
/*e: function bezierpts */

/*s: function _bezsplinepts */
static void
_bezsplinepts(Plist *l, Point *pt, int npt)
{
    Point *p, *ep;
    Point a, b, c, d;
    int periodic;

    if(npt<3)
        return;
    ep = &pt[npt-3];
    periodic = eqpt(pt[0], ep[2]);
    if(periodic){
        a = divpt(addpt(ep[1], pt[0]), 2);
        b = divpt(addpt(ep[1], mulpt(pt[0], 5)), 6);
        c = divpt(addpt(mulpt(pt[0], 5), pt[1]), 6);
        d = divpt(addpt(pt[0], pt[1]), 2);
        bpts(l, a, b, c, d);
    }
    for(p=pt; p<=ep; p++){
        if(p==pt && !periodic){
            a = p[0];
            b = divpt(addpt(p[0], mulpt(p[1], 2)), 3);
        }
        else{
            a = divpt(addpt(p[0], p[1]), 2);
            b = divpt(addpt(p[0], mulpt(p[1], 5)), 6);
        }
        if(p==ep && !periodic){
            c = divpt(addpt(mulpt(p[1], 2), p[2]), 3);
            d = p[2];
        }
        else{
            c = divpt(addpt(mulpt(p[1], 5), p[2]), 6);
            d = divpt(addpt(p[1], p[2]), 2);
        }
        bpts(l, a, b, c, d);
    }
    appendpt(l, d);
}
/*e: function _bezsplinepts */

/*s: function bezsplinepts */
int
bezsplinepts(Point *pt, int npt, Point **pp)
{
    Plist l;
    l.np = 0;
    l.p = nil;
    _bezsplinepts(&l, pt, npt);
    *pp  = l.p;
    return l.np;
}
/*e: function bezsplinepts */

/*s: function bezier */
error0
bezier(Image *dst, Point p0, Point p1, Point p2, Point p3, int end0, int end1, int radius, Image *src, Point sp)
{
    return bezierop(dst, p0, p1, p2, p3, end0, end1, radius, src, sp, SoverD);
}
/*e: function bezier */

/*s: function bezierop */
error0
bezierop(Image *dst, Point p0, Point p1, Point p2, Point p3, int end0, int end1, int radius, Image *src, Point sp, Drawop op)
{
    Plist l;

    l.np = 0;
    bezierpts(&l, p0, p1, p2, p3);
    if(l.np == -1)
        return ERROR_0;
    if(l.np != 0){
        polyop(dst, l.p, l.np, end0, end1, radius, src, addpt(subpt(sp, p0), l.p[0]), op);
        free(l.p);
    }
    return OK_1;
}
/*e: function bezierop */

/*s: function bezspline */
error0
bezspline(Image *dst, Point *pt, int npt, int end0, int end1, int radius, Image *src, Point sp)
{
    return bezsplineop(dst, pt, npt, end0, end1, radius, src, sp, SoverD);
}
/*e: function bezspline */

/*s: function bezsplineop */
error0
bezsplineop(Image *dst, Point *pt, int npt, int end0, int end1, int radius, Image *src, Point sp, Drawop op)
{
    Plist l;

    l.np = 0;
    _bezsplinepts(&l, pt, npt);
    if(l.np==-1)
        return ERROR_0;
    if(l.np != 0){
        polyop(dst, l.p, l.np, end0, end1, radius, src, addpt(subpt(sp, pt[0]), l.p[0]), op);
        free(l.p);
    }
    return OK_1;
}
/*e: function bezsplineop */

/*s: function fillbezier */
error0
fillbezier(Image *dst, Point p0, Point p1, Point p2, Point p3, int w, Image *src, Point sp)
{
    return fillbezierop(dst, p0, p1, p2, p3, w, src, sp, SoverD);
}
/*e: function fillbezier */

/*s: function fillbezierop */
error0
fillbezierop(Image *dst, Point p0, Point p1, Point p2, Point p3, int w, Image *src, Point sp, Drawop op)
{
    Plist l;

    l.np = 0;
    bezierpts(&l, p0, p1, p2, p3);
    if(l.np == -1)
        return ERROR_0;
    if(l.np != 0){
        fillpolyop(dst, l.p, l.np, w, src, addpt(subpt(sp, p0), l.p[0]), op);
        free(l.p);
    }
    return OK_1;
}
/*e: function fillbezierop */

/*s: function fillbezspline */
error0
fillbezspline(Image *dst, Point *pt, int npt, int w, Image *src, Point sp)
{
    return fillbezsplineop(dst, pt, npt, w, src, sp, SoverD);
}
/*e: function fillbezspline */

/*s: function fillbezsplineop */
error0
fillbezsplineop(Image *dst, Point *pt, int npt, int w, Image *src, Point sp, Drawop op)
{
    Plist l;

    l.np = 0;
    _bezsplinepts(&l, pt, npt);
    if(l.np == -1)
        return ERROR_0;
    if(l.np > 0){
        fillpolyop(dst, l.p, l.np, w, src, addpt(subpt(sp, pt[0]), l.p[0]), op);
        free(l.p);
    }
    return OK_1;
}
/*e: function fillbezsplineop */
/*e: lib_graphics/libdraw/bezier.c */
