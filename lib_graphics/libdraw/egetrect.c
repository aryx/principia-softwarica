/*s: lib_graphics/libdraw/egetrect.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <cursor.h>
#include <event.h>

/*s: constant [[W]] */
#define	W	Borderwidth
/*e: constant [[W]] */

/*s: global [[tmp]] */
static Image *tmp[4];
/*e: global [[tmp]] */
/*s: global [[red]] */
static Image *red;
/*e: global [[red]] */

/*s: global [[sweep]] */
static Cursor sweep={
    {-7, -7},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x07,
     0xE0, 0x07, 0xE0, 0x07, 0xE3, 0xF7, 0xE3, 0xF7,
     0xE3, 0xE7, 0xE3, 0xF7, 0xE3, 0xFF, 0xE3, 0x7F,
     0xE0, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,},
    {0x00, 0x00, 0x7F, 0xFE, 0x40, 0x02, 0x40, 0x02,
     0x40, 0x02, 0x40, 0x02, 0x40, 0x02, 0x41, 0xE2,
     0x41, 0xC2, 0x41, 0xE2, 0x41, 0x72, 0x40, 0x38,
     0x40, 0x1C, 0x40, 0x0E, 0x7F, 0xE6, 0x00, 0x00,}
};
/*e: global [[sweep]] */

/*s: function [[brects]] */
static
void
brects(Rectangle r, Rectangle rp[4])
{
    if(Dx(r) < 2*W)
        r.max.x = r.min.x+2*W;
    if(Dy(r) < 2*W)
        r.max.y = r.min.y+2*W;
    rp[0] = Rect(r.min.x, r.min.y, r.max.x, r.min.y+W);
    rp[1] = Rect(r.min.x, r.max.y-W, r.max.x, r.max.y);
    rp[2] = Rect(r.min.x, r.min.y+W, r.min.x+W, r.max.y-W);
    rp[3] = Rect(r.max.x-W, r.min.y+W, r.max.x, r.max.y-W);
}
/*e: function [[brects]] */

/*s: function [[egetrect]] */
Rectangle
egetrect(int but, Mouse *m)
{
    Rectangle r, rc;

    but = 1<<(but-1);
    esetcursor(&sweep);
    while(m->buttons)
        *m = emouse();
    while(!(m->buttons & but)){
        *m = emouse();
        if(m->buttons & (7^but))
            goto Return;
    }
    r.min = m->xy;
    r.max = m->xy;
    do{
        rc = canonrect(r);
        edrawgetrect(rc, 1);
        *m = emouse();
        edrawgetrect(rc, 0);
        r.max = m->xy;
    }while(m->buttons == but);

    Return:
    esetcursor(0);
    if(m->buttons & (7^but)){
        rc.min.x = rc.max.x = 0;
        rc.min.y = rc.max.y = 0;
        while(m->buttons)
            *m = emouse();
    }
    return rc;
}
/*e: function [[egetrect]] */

/*s: function [[freetmp]] */
static
void
freetmp(void)
{
    freeimage(tmp[0]);
    freeimage(tmp[1]);
    freeimage(tmp[2]);
    freeimage(tmp[3]);
    freeimage(red);
    tmp[0] = tmp[1] = tmp[2] = tmp[3] = red = nil;
}
/*e: function [[freetmp]] */

/*s: function [[edrawgetrect]] */
void
edrawgetrect(Rectangle rc, int up)
{
    int i;
    Rectangle r, rects[4];

    if(up && tmp[0]!=nil)
        if(Dx(tmp[0]->r)<Dx(rc) || Dy(tmp[2]->r)<Dy(rc))
            freetmp();

    if(tmp[0] == 0){
        r = Rect(0, 0, Dx(view->r), W);
        tmp[0] = allocimage(display, r, view->chan, 0, -1);
        tmp[1] = allocimage(display, r, view->chan, 0, -1);
        r = Rect(0, 0, W, Dy(view->r));
        tmp[2] = allocimage(display, r, view->chan, 0, -1);
        tmp[3] = allocimage(display, r, view->chan, 0, -1);
        red = allocimage(display, Rect(0,0,1,1), view->chan, 1, DRed);
        if(tmp[0]==0 || tmp[1]==0 || tmp[2]==0 || tmp[3]==0 || red==0)
            drawerror(display, "getrect: allocimage failed");
    }
    brects(rc, rects);
    if(!up){
        for(i=0; i<4; i++)
            draw(view, rects[i], tmp[i], nil, ZP);
        return;
    }
    for(i=0; i<4; i++){
        draw(tmp[i], Rect(0, 0, Dx(rects[i]), Dy(rects[i])), view, nil, rects[i].min);
        draw(view, rects[i], red, nil, ZP);
    }
}
/*e: function [[edrawgetrect]] */
/*e: lib_graphics/libdraw/egetrect.c */
