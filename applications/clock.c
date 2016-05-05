/*s: windows/apps/clock.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <window.h>
#include <event.h>

static Image *hrhand, *minhand;
static Image *dots, *back;

/*s: function circlept */
static Point
circlept(Point c, int r, int degrees)
{

    double rad;
    rad = (double) degrees * PI/180.0;
    c.x += cos(rad)*r;
    c.y -= sin(rad)*r;
    return c;
}
/*e: function circlept */

/*s: function redraw */
static void
redraw(Image *view)
{
    static int tm, ntm;
    static Rectangle r;
    static Point c;
    static int rad;
    static Image *im;
    int i;
    int anghr, angmin;
    static Tm tms;
    static Tm ntms;

    ntm = time(0);
    if(ntm == tm && eqrect(view->r, r))
        return;

    ntms = *localtime(ntm);
    anghr = 90-(ntms.hour*5 + ntms.min/12)*6;
    angmin = 90-ntms.min*6;
    tm = ntm;
    tms = ntms;
    r = view->r;
    c = divpt(addpt(r.min, r.max), 2);
    rad = Dx(r) < Dy(r) ? Dx(r) : Dy(r);
    rad /= 2;
    rad -= 8;

    draw(view, view->r, back, nil, ZP);
    for(i=0; i<12; i++)
        fillellipse(view, circlept(c, rad, i*(360/12)), 2, 2, dots, ZP);

    line(view, c, circlept(c, (rad*3)/4, angmin), 0, 0, 1, minhand, ZP);
    line(view, c, circlept(c, rad/2, anghr), 0, 0, 1, hrhand, ZP);

    flushimage(display, 1);
}
/*e: function redraw */

/*s: function eresized */
void
eresized(int new)
{
    if(new && getwindow(display, Refnone) < 0)
        fprint(2,"can't reattach to window");
    redraw(view);
}
/*e: function eresized */

/*s: function main (windows/apps/clock.c) */
void
main(int, char**)
{
    Event e;
    Mouse m;
    Menu menu;
    char *mstr[] = {"exit", 0};
    int key, timer;
    int t;

    if (initdraw(0, 0, "clock") < 0)
        sysfatal("initdraw failed");
    back = allocimagemix(display, DPalebluegreen, DWhite);

    hrhand = allocimage(display, Rect(0,0,1,1), CMAP8, 1, DDarkblue);
    minhand = allocimage(display, Rect(0,0,1,1), CMAP8, 1, DPaleblue);
    dots = allocimage(display, Rect(0,0,1,1), CMAP8, 1, DBlue);
    redraw(view);

    einit(Emouse);
    t = (30*1000);
    timer = etimer(0, t);

    menu.item = mstr;
    menu.lasthit = 0;
        // the event loop
    for(;;) {
        key = event(&e);
        if(key == Emouse) {
            m = e.mouse;
            if(m.buttons & 4) {
                if(emenuhit(3, &m, &menu) == 0)
                    exits(0);
            }
        } else if(key == timer) {
            redraw(view);
        }
    }	
}
/*e: function main (windows/apps/clock.c) */
/*e: windows/apps/clock.c */
