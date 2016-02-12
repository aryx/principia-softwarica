/*s: lib_graphics/libdraw/border.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function border */
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
/*e: function border */
/*e: lib_graphics/libdraw/border.c */
