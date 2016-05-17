/*s: windows/rio/wm.c */
#include <u.h>
#include <libc.h>

// for dat.h
#include <draw.h>
#include <mouse.h>
#include <cursor.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>
#include <thread.h>

#include <window.h>

#include "dat.h"
#include "fns.h"

// Most of the functions in this file are executed from
// a threadmouse() context (via button3menu()). They send
// a Wctlmesg to the window thread to get the actual modifications
// done on the global windows state.

//----------------------------------------------------------------------------
// Menu
//----------------------------------------------------------------------------

/*s: enum _anon_ (windows/rio/rio.c) */
enum
{
    New,

    Reshape,
    Move,
    Delete,
    Hide,

    Exit,

    Hidden,
};
/*e: enum _anon_ (windows/rio/rio.c) */

/*s: global menu3str */
char*		menu3str[100] = {
 [New]		"New",
 [Reshape]	"Resize",
 [Move]		"Move",
 [Delete]	"Delete",
 [Hide]		"Hide",
 [Exit]		"Exit",
 nil
};
/*e: global menu3str */

/*s: global menu3 */
Menu menu3 = { menu3str };
/*e: global menu3 */


//----------------------------------------------------------------------------
// Helpers
//----------------------------------------------------------------------------

/*s: function goodrect */
/*
 * Check that newly created window will be of manageable size
 */
int
goodrect(Rectangle r)
{
    if(!eqrect(canonrect(r), r))
        return 0;
    if(Dx(r)<100 || Dy(r)<3*font->height)
        return 0;
    /* must have some screen and border visible so we can move it out of the way */
    if(Dx(r) >= Dx(view->r) && Dy(r) >= Dy(view->r))
        return 0;
    /* reasonable sizes only please */
    if(Dx(r) > BIG*Dx(view->r))
        return 0;
    if(Dy(r) > BIG*Dx(view->r))
        return 0;
    return 1;
}
/*e: function goodrect */


/*s: function portion */
int
portion(int x, int lo, int hi)
{
    x -= lo;
    hi -= lo;

    if(x < 20)
        return 0; // top
    if(x > hi-20)
        return 2; // below
    return 1; // middle
}
/*e: function portion */

/*s: function whichcorner */
int
whichcorner(Window *w, Point p)
{
    int i, j;
    
    i = portion(p.x, w->screenr.min.x, w->screenr.max.x);
    j = portion(p.y, w->screenr.min.y, w->screenr.max.y);
    return 3*j + i;
}
/*e: function whichcorner */

/*s: function cornercursor */
void
cornercursor(Window *w, Point p, bool force)
{
    if(w != nil && winborder(w, p))
        riosetcursor(corners[whichcorner(w, p)], force);
    else
        wsetcursor(w, force);
}
/*e: function cornercursor */



//----------------------------------------------------------------------------
// Mouse actions
//----------------------------------------------------------------------------

/*s: function pointto */
Window*
pointto(bool wait)
{
    Window *w;

    menuing = true;
    riosetcursor(&sightcursor, true);

    while(mouse->buttons == 0)
        readmouse(mousectl);

    if(mouse->buttons == 4)
        w = wpointto(mouse->xy);
    else
        w = nil;

    if(wait){
        while(mouse->buttons){
            if(mouse->buttons!=4 && w != nil){	/* cancel */
                cornercursor(input, mouse->xy, false);
                w = nil;
            }
            readmouse(mousectl);
        }
        if(w != nil && wpointto(mouse->xy) != w)
            w = nil;
    }

    cornercursor(input, mouse->xy, false);
    moveto(mousectl, mouse->xy);	/* force cursor update; ugly */
    menuing = false;
    return w;
}
/*e: function pointto */




/*s: function onscreen */
Point
onscreen(Point p)
{
    p.x = max(view->clipr.min.x, p.x);
    p.x = min(view->clipr.max.x, p.x);
    p.y = max(view->clipr.min.y, p.y);
    p.y = min(view->clipr.max.y, p.y);
    return p;
}
/*e: function onscreen */

/*s: function sweep */
Image*
sweep(void)
{
    Image *i, *oi;
    Rectangle r;
    Point p0, p;

    i = nil;

    menuing = true;
    riosetcursor(&crosscursor, true);
    while(mouse->buttons == 0)
        readmouse(mousectl);

    p0 = onscreen(mouse->xy);
    p = p0;
    r = Rpt(p0, p0);
    oi = nil;

    while(mouse->buttons == 4){ // right click
        readmouse(mousectl);

        if(mouse->buttons != 4 && mouse->buttons != 0)
            break;
        if(!eqpt(mouse->xy, p)){
            p = onscreen(mouse->xy);
            r = canonrect(Rpt(p0, p));

            if(Dx(r)>5 && Dy(r)>5){
                i = allocwindow(wscreen, r, Refnone, 0xEEEEEEFF); /* grey */
                freeimage(oi);
                /*s: [[sweep()]] sanity check i */
                if(i == nil)
                    goto Rescue;
                /*e: [[sweep()]] sanity check i */
                oi = i;
                border(i, r, Selborder, red, ZP);
                flushimage(display, true);
            }
        }
    }
    /*s: [[sweep()]] sanity check mouse buttons, i, and rectangle size */
    if(mouse->buttons != 0)
        goto Rescue;
    if(i==nil || Dx(i->r)<100 || Dy(i->r)<3*font->height)
        goto Rescue;
    /*e: [[sweep()]] sanity check mouse buttons, i, and rectangle size */
    oi = i;
    i = allocwindow(wscreen, oi->r, Refbackup, DWhite);
    freeimage(oi);
    /*s: [[sweep()]] sanity check i */
    if(i == nil)
        goto Rescue;
    /*e: [[sweep()]] sanity check i */
    border(i, r, Selborder, red, ZP);
    cornercursor(input, mouse->xy, true);
    goto Return;
/*s: [[sweep()]] Rescue handler */
 Rescue:
    freeimage(i);
    i = nil;
    cornercursor(input, mouse->xy, 1);
    while(mouse->buttons)
        readmouse(mousectl);
/*e: [[sweep()]] Rescue handler */

 Return:
    moveto(mousectl, mouse->xy);	/* force cursor update; ugly */
    menuing = false;
    return i;
}
/*e: function sweep */






/*s: function drawedge */
void
drawedge(Image **bp, Rectangle r)
{
    Image *b = *bp;
    if(b != nil && Dx(b->r) == Dx(r) && Dy(b->r) == Dy(r))
        originwindow(b, r.min, r.min);
    else{
        freeimage(b);
        *bp = allocwindow(wscreen, r, Refbackup, DRed);
    }
}
/*e: function drawedge */

/*s: function drawborder */
void
drawborder(Rectangle r, bool show)
{
    static Image *b[4];
    int i;
    if(!show){
        for(i = 0; i < 4; i++){
            freeimage(b[i]);
            b[i] = nil;
        }
    }else{
        r = canonrect(r);
        drawedge(&b[0], Rect(r.min.x, r.min.y, r.min.x+Borderwidth, r.max.y));
        drawedge(&b[1], Rect(r.min.x+Borderwidth, r.min.y, r.max.x-Borderwidth, r.min.y+Borderwidth));
        drawedge(&b[2], Rect(r.max.x-Borderwidth, r.min.y, r.max.x, r.max.y));
        drawedge(&b[3], Rect(r.min.x+Borderwidth, r.max.y-Borderwidth, r.max.x-Borderwidth, r.max.y));
    }
}
/*e: function drawborder */

/*s: function drag */
Image*
drag(Window *w, Rectangle *rp)
{
    Image *i;
    Image *ni = nil;
    Point p, op, d, dm, om;
    Rectangle r;

    i = w->i;

    menuing = true;
    om = mouse->xy;
    riosetcursor(&boxcursor, true);

    dm = subpt(mouse->xy, w->screenr.min);
    d = subpt(i->r.max, i->r.min);
    op = subpt(mouse->xy, dm);

    drawborder(Rect(op.x, op.y, op.x+d.x, op.y+d.y), true);
    flushimage(display, true);

    while(mouse->buttons == 4){
        p = subpt(mouse->xy, dm);
        if(!eqpt(p, op)){
            // will move previously drawn rectangle thx to originwindow
            drawborder(Rect(p.x, p.y, p.x+d.x, p.y+d.y), true);
            flushimage(display, true);
            op = p;
        }
        readmouse(mousectl);
    }

    r = Rect(op.x, op.y, op.x+d.x, op.y+d.y);
    drawborder(r, false);

    cornercursor(w, mouse->xy, true);
    moveto(mousectl, mouse->xy);	/* force cursor update; ugly */
    menuing = false;

    flushimage(display, true);
    if(mouse->buttons == 0)
        ni=allocwindow(wscreen, r, Refbackup, DWhite);
    /*s: [[drag()]] sanity check mouse buttons and ni */
    if(mouse->buttons!=0 || ni==nil){
        moveto(mousectl, om);
        while(mouse->buttons)
            readmouse(mousectl);
        *rp = Rect(0, 0, 0, 0);
        return nil;
    }
    /*e: [[drag()]] sanity check mouse buttons and ni */
    draw(ni, ni->r, i, nil, i->r.min);
    *rp = r;
    return ni;
}
/*e: function drag */



/*s: function cornerpt */
Point
cornerpt(Rectangle r, Point p, int which)
{
    switch(which){
    case 0:	/* top left */
        p = Pt(r.min.x, r.min.y);
        break;
    case 2:	/* top right */
        p = Pt(r.max.x,r.min.y);
        break;
    case 6:	/* bottom left */
        p = Pt(r.min.x, r.max.y);
        break;
    case 8:	/* bottom right */
        p = Pt(r.max.x, r.max.y);
        break;
    case 1:	/* top edge */
        p = Pt(p.x,r.min.y);
        break;
    case 5:	/* right edge */
        p = Pt(r.max.x, p.y);
        break;
    case 7:	/* bottom edge */
        p = Pt(p.x, r.max.y);
        break;
    case 3:		/* left edge */
        p = Pt(r.min.x, p.y);
        break;
    }
    return p;
}
/*e: function cornerpt */

/*s: function whichrect */
Rectangle
whichrect(Rectangle r, Point p, int which)
{
    switch(which){
    case 0:	/* top left */
        r = Rect(p.x, p.y, r.max.x, r.max.y);
        break;
    case 2:	/* top right */
        r = Rect(r.min.x, p.y, p.x, r.max.y);
        break;
    case 6:	/* bottom left */
        r = Rect(p.x, r.min.y, r.max.x, p.y);
        break;
    case 8:	/* bottom right */
        r = Rect(r.min.x, r.min.y, p.x, p.y);
        break;
    case 1:	/* top edge */
        r = Rect(r.min.x, p.y, r.max.x, r.max.y);
        break;
    case 5:	/* right edge */
        r = Rect(r.min.x, r.min.y, p.x, r.max.y);
        break;
    case 7:	/* bottom edge */
        r = Rect(r.min.x, r.min.y, r.max.x, p.y);
        break;
    case 3:		/* left edge */
        r = Rect(p.x, r.min.y, r.max.x, r.max.y);
        break;
    }
    return canonrect(r);
}
/*e: function whichrect */

/*s: function bandsize */
Image*
bandsize(Window *w)
{
    Image *i;
    Rectangle r, or;
    Point p, startp;
    int which, but;

    p = mouse->xy;
    but = mouse->buttons;
    which = whichcorner(w, p);
    p = cornerpt(w->screenr, p, which);
    wmovemouse(w, p);

    readmouse(mousectl);
    r = whichrect(w->screenr, p, which);
    drawborder(r, true);

    or = r;
    startp = p;
    
    while(mouse->buttons == but){
        p = onscreen(mouse->xy);
        r = whichrect(w->screenr, p, which);
        if(!eqrect(r, or) && goodrect(r)){
            drawborder(r, true);
            flushimage(display, true);
            or = r;
        }
        readmouse(mousectl);
    }

    p = mouse->xy;
    drawborder(or, false);
    flushimage(display, true);

    wsetcursor(w, true);
    /*s: [[bandsize()]] sanity check mouse buttons, rectanglr [[or]], point [[p]] */
    if(mouse->buttons!=0 || Dx(or)<100 || Dy(or)<3*font->height){
        while(mouse->buttons)
            readmouse(mousectl);
        return nil;
    }
    if(abs(p.x - startp.x) + abs(p.y - startp.y) <= 1)
        return nil;
    /*e: [[bandsize()]] sanity check mouse buttons, rectanglr [[or]], point [[p]] */
    i = allocwindow(wscreen, or, Refbackup, DWhite);
    /*s: [[bandsize()]] sanity check i */
    if(i == nil)
        return nil;
    /*e: [[bandsize()]] sanity check i */
    border(i, r, Selborder, red, ZP);
    return i;
}
/*e: function bandsize */


//----------------------------------------------------------------------------
// Window management
//----------------------------------------------------------------------------

/*s: function delete */
void
delete(void)
{
    Window *w;

    w = pointto(true);
    if(w)
        wsendctlmesg(w, Deleted, ZR, nil);
}
/*e: function delete */

/*s: function resize */
void
resize(void)
{
    Window *w;
    Image *i;

    w = pointto(true);
    if(w == nil)
        return;
    i = sweep();
    if(i)
        wsendctlmesg(w, Reshaped, i->r, i);
}
/*e: function resize */

/*s: function move */
void
move(void)
{
    Window *w;
    Image *i;
    Rectangle r;

    w = pointto(false);
    if(w == nil)
        return;
    i = drag(w, &r);
    if(i)
        wsendctlmesg(w, Moved, r, i);
    cornercursor(input, mouse->xy, true);
}
/*e: function move */

/*s: function whide */
int
whide(Window *w)
{
    Image *i;
    int j;

    for(j=0; j<nhidden; j++)
        if(hidden[j] == w)	/* already hidden */
            return -1;

    i = allocimage(display, w->screenr, w->i->chan, false, DWhite);
    if(i){
        hidden[nhidden++] = w;
        wsendctlmesg(w, Reshaped, ZR, i);
        return 1;
    }
    return 0;
}
/*e: function whide */

/*s: function wunhide */
int
wunhide(int h)
{
    Image *i;
    Window *w;

    w = hidden[h];
    i = allocwindow(wscreen, w->i->r, Refbackup, DWhite);
    if(i){
        --nhidden;
        memmove(hidden+h, hidden+h+1, (nhidden-h)*sizeof(Window*));
        wsendctlmesg(w, Reshaped, w->i->r, i);
        return 1;
    }
    return 0;
}
/*e: function wunhide */

/*s: function hide */
void
hide(void)
{
    Window *w;

    w = pointto(true);
    if(w == nil)
        return;
    whide(w);
}
/*e: function hide */

/*s: function unhide */
void
unhide(int h)
{
    Window *w;

    h -= Hidden;
    w = hidden[h];
    if(w == nil)
        return;
    wunhide(h);
}
/*e: function unhide */




/*s: global rcargv */
char *rcargv[] = { "rc", "-i", nil };
/*e: global rcargv */


/*s: function new */
Window*
new(Image *i, bool hideit, bool scrollit, int pid, char *dir, char *cmd, char **argv)
{
    Channel *cm, *ck, *cctl;
    Channel *cpid;
    Mousectl *mc;
    Window *w;
    /*s: [[new()]] other locals */
    void **arg;
    /*e: [[new()]] other locals */

    /*s: [[new()]] sanity check i */
    if(i == nil)
        return nil;
    /*e: [[new()]] sanity check i */

    /*s: [[new()]] channels creation */
    cm = chancreate(sizeof(Mouse), 0);
    ck = chancreate(sizeof(Rune*), 0);
    cctl = chancreate(sizeof(Wctlmesg), 4);
    /*e: [[new()]] channels creation */
    cpid = chancreate(sizeof(int), 0);
    /*s: [[new()]] sanity check channels */
    if(cm==nil || ck==nil || cctl==nil)
        error("new: channel alloc failed");
    /*e: [[new()]] sanity check channels */

    /*s: [[new()]] mc allocation */
    mc = emalloc(sizeof(Mousectl));
    *mc = *mousectl;
    mc->image = i;
    mc->c = cm;
    /*e: [[new()]] mc allocation */

    w = wmk(i, mc, ck, cctl, scrollit);
    free(mc);	/* wmk copies *mc */

    // growing array
    windows = erealloc(windows, ++nwindow * sizeof(Window*));
    windows[nwindow-1] = w;
    /*s: [[new()]] if hideit */
    if(hideit){
        hidden[nhidden++] = w;
        w->screenr = ZR;
    }
    /*e: [[new()]] if hideit */

    // a new thread! for this new window!
    threadcreate(winctl, w, 8192);

    if(!hideit)
        wcurrent(w);

    flushimage(display, true);

    /*s: [[new()]] if pid == 0, create winshell process and set pid */
    if(pid == 0){
        arg = emalloc(5 * sizeof(void*));
        arg[0] = w;
        arg[1] = cpid;
        arg[2] = cmd;
        if(argv == nil)
            arg[3] = rcargv;
        else
            arg[3] = argv;
        arg[4] = dir;

        proccreate(winshell, arg, 8192);

        pid = recvul(cpid);
        free(arg);
    }
    /*e: [[new()]] if pid == 0, create winshell process and set pid */
    /*s: [[new()]] sanity check pid */
    if(pid == 0){
        /* window creation failed */
        wsendctlmesg(w, Deleted, ZR, nil);
        chanfree(cpid);
        return nil;
    }
    /*e: [[new()]] sanity check pid */

    wsetpid(w, pid, true);
    wsetname(w);

    if(dir)
        w->dir = estrdup(dir);

    chanfree(cpid);
    return w;
}
/*e: function new */


//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------


/*s: function button3menu */
void
button3menu(void)
{
    int i;

    /*s: [[button3menu()]] menu3str adjustments with hidden windows */
    for(i=0; i<nhidden; i++)
        menu3str[i+Hidden] = hidden[i]->label;
    menu3str[i+Hidden] = nil;
    /*e: [[button3menu()]] menu3str adjustments with hidden windows */

    sweeping = true;
    switch(i = menuhit(3, mousectl, &menu3, wscreen)){
    /*s: [[button3menu()]] cases */
    case Exit:
        send(exitchan, nil);
        break;
    /*x: [[button3menu()]] cases */
    case New:
        new(sweep(), false, scrolling, 0, nil, "/bin/rc", nil);
        break;
    /*x: [[button3menu()]] cases */
    case Delete:
        delete();
        break;
    /*x: [[button3menu()]] cases */
    case Move:
        move();
        break;
    /*x: [[button3menu()]] cases */
    case Reshape:
        resize();
        break;
    /*x: [[button3menu()]] cases */
    case Hide:
        hide();
        break;
    /*x: [[button3menu()]] cases */
    default:
        unhide(i);
        break;
    /*e: [[button3menu()]] cases */
    case -1:
        break;
    }
    sweeping = false;
}
/*e: function button3menu */

/*e: windows/rio/wm.c */
