/*s: lib_graphics/libdraw/drawrepl.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

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

/*e: lib_graphics/libdraw/drawrepl.c */
