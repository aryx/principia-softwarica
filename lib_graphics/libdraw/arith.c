/*s: lib_graphics/libdraw/arith.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function Pt */
Point
Pt(int x, int y)
{
    Point p;

    p.x = x;
    p.y = y;
    return p;
}
/*e: function Pt */

/*s: function Rect */
Rectangle
Rect(int x, int y, int bx, int by)
{
    Rectangle r;

    r.min.x = x;
    r.min.y = y;
    r.max.x = bx;
    r.max.y = by;
    return r;
}
/*e: function Rect */

/*s: function Rpt */
Rectangle
Rpt(Point min, Point max)
{
    Rectangle r;

    r.min = min;
    r.max = max;
    return r;
}
/*e: function Rpt */

/*s: function addpt */
Point
addpt(Point a, Point b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}
/*e: function addpt */

/*s: function subpt */
Point
subpt(Point a, Point b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}
/*e: function subpt */

/*s: function insetrect */
Rectangle
insetrect(Rectangle r, int n)
{
    r.min.x += n;
    r.min.y += n;
    r.max.x -= n;
    r.max.y -= n;
    return r;
}
/*e: function insetrect */

/*s: function divpt */
Point
divpt(Point a, int b)
{
    a.x /= b;
    a.y /= b;
    return a;
}
/*e: function divpt */

/*s: function mulpt */
Point
mulpt(Point a, int b)
{
    a.x *= b;
    a.y *= b;
    return a;
}
/*e: function mulpt */

/*s: function rectsubpt */
Rectangle
rectsubpt(Rectangle r, Point p)
{
    r.min.x -= p.x;
    r.min.y -= p.y;
    r.max.x -= p.x;
    r.max.y -= p.y;
    return r;
}
/*e: function rectsubpt */

/*s: function rectaddpt */
Rectangle
rectaddpt(Rectangle r, Point p)
{
    r.min.x += p.x;
    r.min.y += p.y;
    r.max.x += p.x;
    r.max.y += p.y;
    return r;
}
/*e: function rectaddpt */

/*s: function eqpt */
int
eqpt(Point p, Point q)
{
    return p.x==q.x && p.y==q.y;
}
/*e: function eqpt */

/*s: function eqrect */
int
eqrect(Rectangle r, Rectangle s)
{
    return r.min.x==s.min.x && r.max.x==s.max.x &&
           r.min.y==s.min.y && r.max.y==s.max.y;
}
/*e: function eqrect */

/*s: function rectXrect */
int
rectXrect(Rectangle r, Rectangle s)
{
    return r.min.x<s.max.x && s.min.x<r.max.x &&
           r.min.y<s.max.y && s.min.y<r.max.y;
}
/*e: function rectXrect */

/*s: function rectinrect */
int
rectinrect(Rectangle r, Rectangle s)
{
    return s.min.x<=r.min.x && r.max.x<=s.max.x && s.min.y<=r.min.y && r.max.y<=s.max.y;
}
/*e: function rectinrect */

/*s: function ptinrect */
int
ptinrect(Point p, Rectangle r)
{
    return p.x>=r.min.x && p.x<r.max.x &&
           p.y>=r.min.y && p.y<r.max.y;
}
/*e: function ptinrect */

/*s: function canonrect */
Rectangle
canonrect(Rectangle r)
{
    int t;
    if (r.max.x < r.min.x) {
        t = r.min.x;
        r.min.x = r.max.x;
        r.max.x = t;
    }
    if (r.max.y < r.min.y) {
        t = r.min.y;
        r.min.y = r.max.y;
        r.max.y = t;
    }
    return r;
}
/*e: function canonrect */

/*s: function combinerect */
void
combinerect(Rectangle *r1, Rectangle r2)
{
    if(r1->min.x > r2.min.x)
        r1->min.x = r2.min.x;
    if(r1->min.y > r2.min.y)
        r1->min.y = r2.min.y;
    if(r1->max.x < r2.max.x)
        r1->max.x = r2.max.x;
    if(r1->max.y < r2.max.y)
        r1->max.y = r2.max.y;
}
/*e: function combinerect */

/*s: global drawld2chan */
ulong drawld2chan[] = {
    GREY1,
    GREY2,
    GREY4,
    CMAP8,
};
/*e: global drawld2chan */

/*s: global log2 */
/*
 * was used by libmemlayer/line.c; made static, instead of deleting it,
 * to avoid updating many binaries on sources.
 */
static int log2[] = { -1, 0, 1, -1, 2, -1, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1, 4, -1, -1, -1, -1, -1, -1, -1, 4 /* BUG */, -1, -1, -1, -1, -1, -1, -1, 5 };
/*e: global log2 */

/*s: function setalpha */
ulong
setalpha(ulong color, uchar alpha)
{
    int red, green, blue;

    red = (color >> 3*8) & 0xFF;
    green = (color >> 2*8) & 0xFF;
    blue = (color >> 1*8) & 0xFF;
    /* ignore incoming alpha */
    red = (red * alpha)/255;
    green = (green * alpha)/255;
    blue = (blue * alpha)/255;
    return (red<<3*8) | (green<<2*8) | (blue<<1*8) | (alpha<<0*8);
}
/*e: function setalpha */

/*s: global ZP */
Point	ZP;
/*e: global ZP */
/*s: global ZR */
Rectangle ZR;
/*e: global ZR */
/*e: lib_graphics/libdraw/arith.c */
