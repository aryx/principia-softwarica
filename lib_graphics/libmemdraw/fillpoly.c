/*s: lib_graphics/libmemdraw/fillpoly.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <memlayer.h>

typedef struct Seg	Seg;

/*s: struct Seg */
struct Seg
{
    Point	p0;
    Point	p1;

    long	d; // direction? 1 or -1?

    /*s: [[Seg]] other fields */
    long	num;
    long	den;
    long	dz;
    long	dzrem;
    /*x: [[Seg]] other fields */
    long	z;
    long	zerr;
    /*e: [[Seg]] other fields */
};
/*e: struct Seg */

static	void	zsort(Seg **seg, Seg **ep);
static	int	ycompare(void*, void*);
static	int	xcompare(void*, void*);
static	int	zcompare(void*, void*);
static	void	xscan(Memimage *dst, Seg **seg, Seg *segtab, int nseg, int wind, Memimage *src, Point sp, int, int, int, int);
static	void	yscan(Memimage *dst, Seg **seg, Seg *segtab, int nseg, int wind, Memimage *src, Point sp, int, int);

/*s: function fillline */
static void
fillline(Memimage *dst, int xleft, int xright, int y, Memimage *src, Point p, int op)
{
    Rectangle r;

    r.min.x = xleft;
    r.min.y = y;
    r.max.x = xright;
    r.max.y = y+1;

    p.x += xleft;
    p.y += y;

    memdraw(dst, r, src, p, memopaque, p, op);
}
/*e: function fillline */

/*s: function fillpoint */
static void
fillpoint(Memimage *dst, int x, int y, Memimage *src, Point p, int op)
{
    Rectangle r;

    r.min.x = x;
    r.min.y = y;
    r.max.x = x+1;
    r.max.y = y+1;
    p.x += x;
    p.y += y;
    memdraw(dst, r, src, p, memopaque, p, op);
}
/*e: function fillpoint */

/*s: function memfillpoly */
void
memfillpoly(Memimage *dst, Point *vert, int nvert, int w, Memimage *src, Point sp, int op)
{
    _memfillpolysc(dst, vert, nvert, w, src, sp, false, 0, false, op);
}
/*e: function memfillpoly */

/*s: function _memfillpolysc */
void
_memfillpolysc(Memimage *dst, Point *vert, int nvert, int w, Memimage *src, Point sp, bool detail, int fixshift, bool clipped, int op)
{
    // array<Seg> (length = nvert+1)
    Seg *segtab;
    // array<Seg*> (length = nvert+2)
    Seg **seg;
    Point p0;
    int i;

    /*s: [[_memfillpolysc()]] sanity check nvert */
    if(nvert == 0)
        return;
    /*e: [[_memfillpolysc()]] sanity check nvert */

    segtab = malloc((nvert+1)*sizeof(Seg));
    /*s: [[_memfillpolysc()]] sanity check segtab */
    if(segtab == nil) {
        free(seg);
        return;
    }
    /*e: [[_memfillpolysc()]] sanity check segtab */
    seg = malloc((nvert+2)*sizeof(Seg*));
    /*s: [[_memfillpolysc()]] sanity check seg */
    if(seg == nil)
        return;
    /*e: [[_memfillpolysc()]] sanity check seg */

    sp.x = (sp.x - vert[0].x) >> fixshift;
    sp.y = (sp.y - vert[0].y) >> fixshift;

    p0 = vert[nvert-1]; // start from the end
    /*s: [[_memfillpolysc()]] adjust p0 if no fixshift */
    if(!fixshift) {
        p0.x <<= 1;
        p0.y <<= 1;
    }
    /*e: [[_memfillpolysc()]] adjust p0 if no fixshift */
    for(i = 0; i < nvert; i++) {
        segtab[i].p0 = p0;
        p0 = vert[i];
        /*s: [[_memfillpolysc()]] adjust p0 if no fixshift */
        if(!fixshift) {
            p0.x <<= 1;
            p0.y <<= 1;
        }
        /*e: [[_memfillpolysc()]] adjust p0 if no fixshift */
        segtab[i].p1 = p0;
        segtab[i].d = 1;
    }
    /*s: [[_memfillpolysc()]] adjust fixshift if no fixshift */
    if(!fixshift)
        fixshift = 1;
    /*e: [[_memfillpolysc()]] adjust fixshift if no fixshift */

    xscan(dst, seg, segtab, nvert, w, src, sp, detail, fixshift, clipped, op);
    /*s: [[_memfillpolysc()]] if detail */
    if(detail)
        yscan(dst, seg, segtab, nvert, w, src, sp, fixshift, op);
    /*e: [[_memfillpolysc()]] if detail */

    free(seg);
    free(segtab);
}
/*e: function _memfillpolysc */

/*s: function mod */
static long
mod(long x, long y)
{
    long z;

    z = x%y;
    if((long)(((ulong)z)^((ulong)y)) > 0 || z == 0)
        return z;
    return z + y;
}
/*e: function mod */

/*s: function sdiv */
static long
sdiv(long x, long y)
{
    if((long)(((ulong)x)^((ulong)y)) >= 0 || x == 0)
        return x/y;

    return (x+((y>>30)|1))/y-1;
}
/*e: function sdiv */

/*s: function smuldivmod */
static long
smuldivmod(long x, long y, long z, long *mod)
{
    vlong vx;

    if(x == 0 || y == 0){
        *mod = 0;
        return 0;
    }
    vx = x;
    vx *= y;
    *mod = vx % z;
    if(*mod < 0)
        *mod += z;	/* z is always >0 */
    if((vx < 0) == (z < 0))
        return vx/z;
    return -((-vx)/z);
}
/*e: function smuldivmod */

/*s: function xscan */
static void
xscan(Memimage *dst, Seg **seg, Seg *segtab, int nseg, int wind, Memimage *src, Point sp, bool detail, int fixshift, bool clipped, int op)
{
    Seg **ep, **next, **p, **q, *s;
    long y, maxy, x, x2, xerr, xden, onehalf;
    long n, i, iy, cnt, ix, ix2, minx, maxx;
    Point pt;
    void	(*fill)(Memimage*, int, int, int, Memimage*, Point, int);

    fill = fillline;
    USED(clipped);

    for(i=0, s=segtab, p=seg; i<nseg; i++, s++) {
        *p = s;
        if(s->p0.y == s->p1.y)
            continue;
        if(s->p0.y > s->p1.y) {
            // swap(s->p0, s->p1)
            pt = s->p0;
            s->p0 = s->p1;
            s->p1 = pt;

            s->d = -s->d;
        }
        s->num = s->p1.x - s->p0.x;
        s->den = s->p1.y - s->p0.y;

        s->dz = sdiv(s->num, s->den) << fixshift;
        s->dzrem = mod(s->num, s->den) << fixshift;
        s->dz += sdiv(s->dzrem, s->den);
        s->dzrem = mod(s->dzrem, s->den);
        p++;
    }
    n = p-seg;
    if(n == 0)
        return;
    *p = nil;
    qsort(seg, n , sizeof(Seg*), ycompare);

    onehalf = 0;
    if(fixshift)
        onehalf = 1 << (fixshift-1);

    minx = dst->clipr.min.x;
    maxx = dst->clipr.max.x;

    y = seg[0]->p0.y;
    if(y < (dst->clipr.min.y << fixshift))
        y = dst->clipr.min.y << fixshift;
    iy = (y + onehalf) >> fixshift;
    y = (iy << fixshift) + onehalf;
    maxy = dst->clipr.max.y << fixshift;

    ep = next = seg;

    while(y<maxy) {
        for(q = p = seg; p < ep; p++) {
            s = *p;
            if(s->p1.y < y)
                continue;
            s->z += s->dz;
            s->zerr += s->dzrem;
            if(s->zerr >= s->den) {
                s->z++;
                s->zerr -= s->den;
                if(s->zerr < 0 || s->zerr >= s->den)
                    print("bad ratzerr1: %ld den %ld dzrem %ld\n", s->zerr, s->den, s->dzrem);
            }
            *q++ = s;
        }

        for(p = next; *p; p++) {
            s = *p;
            if(s->p0.y >= y)
                break;
            if(s->p1.y < y)
                continue;
            s->z = s->p0.x;
            s->z += smuldivmod(y - s->p0.y, s->num, s->den, &s->zerr);
            if(s->zerr < 0 || s->zerr >= s->den)
                print("bad ratzerr2: %ld den %ld ratdzrem %ld\n", s->zerr, s->den, s->dzrem);
            *q++ = s;
        }
        ep = q;
        next = p;

        if(ep == seg) {
            if(*next == 0)
                break;
            iy = (next[0]->p0.y + onehalf) >> fixshift;
            y = (iy << fixshift) + onehalf;
            continue;
        }

        zsort(seg, ep);

        for(p = seg; p < ep; p++) {
            cnt = 0;

            x = p[0]->z;
            xerr = p[0]->zerr;
            xden = p[0]->den;

            ix = (x + onehalf) >> fixshift;
            if(ix >= maxx)
                break;
            if(ix < minx)
                ix = minx;

            cnt += p[0]->d;
            p++;
            for(;;) {
                if(p == ep) {
                    print("xscan: fill to infinity");
                    return;
                }
                cnt += p[0]->d;
                if((cnt & wind) == 0)
                    break;
                p++;
            }

            x2 = p[0]->z;
            ix2 = (x2 + onehalf) >> fixshift;
            if(ix2 <= minx)
                continue;
            if(ix2 > maxx)
                ix2 = maxx;

            /*s: [[xscan()]] if detail */
            if(ix == ix2 && detail) {
                if(xerr*p[0]->den + p[0]->zerr*xden > p[0]->den*xden)
                    x++;
                ix = (x + x2) >> (fixshift+1);
                ix2 = ix+1;
            }
            /*e: [[xscan()]] if detail */

            // the call
            (*fill)(dst, ix, ix2, iy, src, sp, op);
        }
        y += (1<<fixshift);
        iy++;
    }
}
/*e: function xscan */

/*s: function yscan */
static void
yscan(Memimage *dst, Seg **seg, Seg *segtab, int nseg, int wind, Memimage *src, Point sp, int fixshift, int op)
{
    long x, maxx, y, y2, yerr, yden, onehalf;
    Seg **ep, **next, **p, **q, *s;
    int n, i, ix, cnt, iy, iy2, miny, maxy;
    Point pt;

    for(i=0, s=segtab, p=seg; i<nseg; i++, s++) {
        *p = s;
        if(s->p0.x == s->p1.x)
            continue;
        if(s->p0.x > s->p1.x) {
            pt = s->p0;
            s->p0 = s->p1;
            s->p1 = pt;
            s->d = -s->d;
        }
        s->num = s->p1.y - s->p0.y;
        s->den = s->p1.x - s->p0.x;
        s->dz = sdiv(s->num, s->den) << fixshift;
        s->dzrem = mod(s->num, s->den) << fixshift;
        s->dz += sdiv(s->dzrem, s->den);
        s->dzrem = mod(s->dzrem, s->den);
        p++;
    }
    n = p-seg;
    if(n == 0)
        return;
    *p = 0;
    qsort(seg, n , sizeof(Seg*), xcompare);

    onehalf = 0;
    if(fixshift)
        onehalf = 1 << (fixshift-1);

    miny = dst->clipr.min.y;
    maxy = dst->clipr.max.y;

    x = seg[0]->p0.x;
    if(x < (dst->clipr.min.x << fixshift))
        x = dst->clipr.min.x << fixshift;
    ix = (x + onehalf) >> fixshift;
    x = (ix << fixshift) + onehalf;
    maxx = dst->clipr.max.x << fixshift;

    ep = next = seg;

    while(x<maxx) {
        for(q = p = seg; p < ep; p++) {
            s = *p;
            if(s->p1.x < x)
                continue;
            s->z += s->dz;
            s->zerr += s->dzrem;
            if(s->zerr >= s->den) {
                s->z++;
                s->zerr -= s->den;
                if(s->zerr < 0 || s->zerr >= s->den)
                    print("bad ratzerr1: %ld den %ld ratdzrem %ld\n", s->zerr, s->den, s->dzrem);
            }
            *q++ = s;
        }

        for(p = next; *p; p++) {
            s = *p;
            if(s->p0.x >= x)
                break;
            if(s->p1.x < x)
                continue;
            s->z = s->p0.y;
            s->z += smuldivmod(x - s->p0.x, s->num, s->den, &s->zerr);
            if(s->zerr < 0 || s->zerr >= s->den)
                print("bad ratzerr2: %ld den %ld ratdzrem %ld\n", s->zerr, s->den, s->dzrem);
            *q++ = s;
        }
        ep = q;
        next = p;

        if(ep == seg) {
            if(*next == 0)
                break;
            ix = (next[0]->p0.x + onehalf) >> fixshift;
            x = (ix << fixshift) + onehalf;
            continue;
        }

        zsort(seg, ep);

        for(p = seg; p < ep; p++) {
            cnt = 0;
            y = p[0]->z;
            yerr = p[0]->zerr;
            yden = p[0]->den;
            iy = (y + onehalf) >> fixshift;
            if(iy >= maxy)
                break;
            if(iy < miny)
                iy = miny;
            cnt += p[0]->d;
            p++;
            for(;;) {
                if(p == ep) {
                    print("yscan: fill to infinity");
                    return;
                }
                cnt += p[0]->d;
                if((cnt&wind) == 0)
                    break;
                p++;
            }
            y2 = p[0]->z;
            iy2 = (y2 + onehalf) >> fixshift;
            if(iy2 <= miny)
                continue;
            if(iy2 > maxy)
                iy2 = maxy;
            if(iy == iy2) {
                if(yerr*p[0]->den + p[0]->zerr*yden > p[0]->den*yden)
                    y++;
                iy = (y + y2) >> (fixshift+1);
                fillpoint(dst, ix, iy, src, sp, op);
            }
        }
        x += (1<<fixshift);
        ix++;
    }
}
/*e: function yscan */

/*s: function zsort */
static void
zsort(Seg **seg, Seg **ep)
{
    int done;
    Seg **q, **p, *s;

    if(ep-seg < 20) {
        /* bubble sort by z - they should be almost sorted already */
        q = ep;
        do {
            done = 1;
            q--;
            for(p = seg; p < q; p++) {
                if(p[0]->z > p[1]->z) {
                    s = p[0];
                    p[0] = p[1];
                    p[1] = s;
                    done = 0;
                }
            }
        } while(!done);
    } else {
        q = ep-1;
        for(p = seg; p < q; p++) {
            if(p[0]->z > p[1]->z) {
                qsort(seg, ep-seg, sizeof(Seg*), zcompare);
                break;
            }
        }
    }
}
/*e: function zsort */

/*s: function ycompare */
static int
ycompare(void *a, void *b)
{
    Seg **s0 = a, **s1 = b;
    long y0, y1;

    y0 = (*s0)->p0.y;
    y1 = (*s1)->p0.y;

    if(y0 < y1)
        return -1;
    if(y0 == y1)
        return 0;
    return 1;
}
/*e: function ycompare */

/*s: function xcompare */
static int
xcompare(void *a, void *b)
{
    Seg **s0, **s1;
    long x0, x1;

    s0 = a;
    s1 = b;
    x0 = (*s0)->p0.x;
    x1 = (*s1)->p0.x;

    if(x0 < x1)
        return -1;
    if(x0 == x1)
        return 0;
    return 1;
}
/*e: function xcompare */

/*s: function zcompare */
static int
zcompare(void *a, void *b)
{
    Seg **s0 = a, **s1 = b;
    long z0, z1;

    z0 = (*s0)->z;
    z1 = (*s1)->z;

    if(z0 < z1)
        return -1;
    if(z0 == z1)
        return 0;
    return 1;
}
/*e: function zcompare */
/*e: lib_graphics/libmemdraw/fillpoly.c */
