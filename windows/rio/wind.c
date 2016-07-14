/*s: windows/rio/wind.c */
#include <u.h>
#include <libc.h>

#include <draw.h>
#include <window.h>
#include <thread.h>
#include <cursor.h>
#include <mouse.h>
#include <keyboard.h>

#include <frame.h>
#include <fcall.h>

#include "dat.h"
#include "fns.h"

/*s: global topped */
static	int		topped;
/*e: global topped */
/*s: global id */
static	int	id;
/*e: global id */

/*s: global cols */
// map<Property, Color>
static	Image	*cols[NCOL];
/*e: global cols */
/*s: global grey */
static	Image	*grey;
/*e: global grey */
/*s: global darkgrey */
static	Image	*darkgrey;
/*e: global darkgrey */


/*s: global titlecol */
static	Image	*titlecol;
/*e: global titlecol */
/*s: global lighttitlecol */
static	Image	*lighttitlecol;
/*e: global lighttitlecol */
/*s: global holdcol */
static	Image	*holdcol;
/*e: global holdcol */
/*s: global lightholdcol */
static	Image	*lightholdcol;
/*e: global lightholdcol */
/*s: global paleholdcol */
static	Image	*paleholdcol;
/*e: global paleholdcol */


/*s: function wsendctlmesg */
void
wsendctlmesg(Window *w, int type, Rectangle r, Image *image)
{
    Wctlmesg wcm;

    wcm.type = type;
    wcm.r = r;
    wcm.image = image;

    send(w->cctl, &wcm);
}
/*e: function wsendctlmesg */


/*s: function wborder */
void
wborder(Window *w, int type)
{
    Image *col;

    /*s: [[wborder()]] sanity check w */
    if(w->i == nil)
        return;
    /*e: [[wborder()]] sanity check w */
    /*s: [[wborder()]] if holding */
    if(w->holding){
        if(type == Selborder)
            col = holdcol;
        else
            col = paleholdcol;
    }
    /*e: [[wborder()]] if holding */
    else{
        if(type == Selborder)
            col = titlecol;
        else
            col = lighttitlecol;
    }

    border(w->i, w->i->r, Selborder, col, ZP);
}
/*e: function wborder */

/*s: function wsetcols */
void
wsetcols(Window *w)
{
    /*s: [[wsetcols()]] if holding */
    if(w->holding)
        if(w == input)
            w->frm.cols[TEXT] = w->frm.cols[HTEXT] = holdcol;
        else
            w->frm.cols[TEXT] = w->frm.cols[HTEXT] = lightholdcol;
    /*e: [[wsetcols()]] if holding */
    else
        if(w == input)
            w->frm.cols[TEXT] = w->frm.cols[HTEXT] = display->black;
        else
            w->frm.cols[TEXT] = w->frm.cols[HTEXT] = darkgrey;
}
/*e: function wsetcols */


/*s: function wmk */
Window*
wmk(Image *i, Mousectl *mc, Channel *ck, Channel *cctl, bool scrolling)
{
    Window *w;
    Rectangle r;

    /*s: [[wmk()]] cols initialisation */
    if(cols[0] == nil){
        /* greys are multiples of 0x11111100+0xFF, 14* being palest */
        grey     = allocimage(display, Rect(0,0,1,1), CMAP8, true, 0xEEEEEEFF);
        darkgrey = allocimage(display, Rect(0,0,1,1), CMAP8, true, 0x666666FF);
        cols[BACK] = display->white;
        cols[HIGH] = allocimage(display, Rect(0,0,1,1), CMAP8, true, 0xCCCCCCFF);
        cols[BORD] = allocimage(display, Rect(0,0,1,1), CMAP8, true, 0x999999FF);
        cols[TEXT] = display->black;
        cols[HTEXT] = display->black;

        titlecol     = allocimage(display, Rect(0,0,1,1), CMAP8, true, DGreygreen);
        lighttitlecol= allocimage(display, Rect(0,0,1,1), CMAP8, true, DPalegreygreen);
        holdcol      = allocimage(display, Rect(0,0,1,1), CMAP8, true, DMedblue);
        lightholdcol = allocimage(display, Rect(0,0,1,1), CMAP8, true, DGreyblue);
        paleholdcol  = allocimage(display, Rect(0,0,1,1), CMAP8, true, DPalegreyblue);
    }
    /*e: [[wmk()]] cols initialisation */

    w = emalloc(sizeof(Window));

    w->i = i;
    w->screenr = i->r;
    w->cursorp = nil;

    w->id = ++id;
    w->topped = ++topped;

    /*s: [[wmk()]] channels creation */
    w->mc = *mc;
    w->ck = ck;
    w->cctl = cctl;
    /*x: [[wmk()]] channels creation */
    w->mouseread =  chancreate(sizeof(Mousereadmesg), 0);
    /*x: [[wmk()]] channels creation */
    w->consread =  chancreate(sizeof(Consreadmesg), 0);
    /*x: [[wmk()]] channels creation */
    w->conswrite = chancreate(sizeof(Conswritemesg), 0);
    /*x: [[wmk()]] channels creation */
    w->wctlread =  chancreate(sizeof(Consreadmesg), 0);
    /*e: [[wmk()]] channels creation */

    /*s: [[wmk()]] textual window settings */
    r = insetrect(i->r, Selborder+1);
    w->scrollr = r;
    w->scrollr.max.x = r.min.x+Scrollwid;

    r.min.x += Scrollwid+Scrollgap;
    frinit(&w->frm, r, font, i, cols);

    w->lastsr = ZR;
    w->frm.maxtab = maxtab * stringwidth(font, "0");
    w->scrolling = scrolling;

    r = insetrect(w->i->r, Selborder);
    draw(w->i, r, cols[BACK], nil, w->frm.entire.min);
    /*e: [[wmk()]] textual window settings */

    w->notefd = -1;
    w->dir = estrdup(startdir);
    w->label = estrdup("<unnamed>");

    wborder(w, Selborder);
    wscrdraw(w);

    incref(w);	/* ref will be removed after mounting; avoids delete before ready to be deleted */
    return w;
}
/*e: function wmk */

/*s: function wsetname */
void
wsetname(Window *w)
{
    int i, n;
    char err[ERRMAX];
    
    n = sprint(w->name, "window.%d.%d", w->id, w->namecount++);

    if(nameimage(w->i, w->name, true) > 0)
        return;
    // else
    /*s: [[wsetname()]] if image name already in use, try another name */
    for(i='A'; i<='Z'; i++){
        if(nameimage(w->i, w->name, true) > 0)
            return;

        errstr(err, sizeof err);
        if(strcmp(err, "image name in use") != 0)
            break;
        w->name[n] = i;
        w->name[n+1] = '\0';
    }
    w->name[0] = 0;
    fprint(STDERR, "rio: setname failed: %s\n", err);
    /*e: [[wsetname()]] if image name already in use, try another name */
}
/*e: function wsetname */

/*s: function wresize */
void
wresize(Window *w, Image *i, bool move)
{
    Rectangle r, or;

    or = w->i->r;
    if(move || (Dx(or)==Dx(i->r) && Dy(or)==Dy(i->r)))
        draw(i, i->r, w->i, nil, w->i->r.min);
    freeimage(w->i);
    w->i = i;
    wsetname(w); // publish new window name by incrementing namecount
    w->mc.image = i;

    /*s: [[wresize()]] textual window updates */
    r = insetrect(i->r, Selborder+1);
    w->scrollr = r;
    w->scrollr.max.x = r.min.x+Scrollwid;

    w->lastsr = ZR;

    r.min.x += Scrollwid+Scrollgap;

    if(move)
        frsetrects(&w->frm, r, w->i);
    else{
        frclear(&w->frm, false);
        frinit(&w->frm, r, w->frm.font, w->i, cols);
        wsetcols(w);

        w->frm.maxtab = maxtab * stringwidth(w->frm.font, "0");

        r = insetrect(w->i->r, Selborder);
        draw(w->i, r, cols[BACK], nil, w->frm.entire.min);

        wfill(w);
        wsetselect(w, w->q0, w->q1);
        wscrdraw(w);
    }
    /*e: [[wresize()]] textual window updates */

    wborder(w, Selborder);
    w->topped = ++topped;

    w->resized = true;
    w->mouse.counter++;
}
/*e: function wresize */

/*s: function wrefresh */
void
wrefresh(Window *w, Rectangle)
{
    Frame *frm = &w->frm;

    /* BUG: rectangle is ignored */
    if(w == input)
        wborder(w, Selborder);
    else
        wborder(w, Unselborder);
    if(w->mouseopen)
        return;
    // else

    draw(w->i, insetrect(w->i->r, Borderwidth), frm->cols[BACK], nil, 
         w->i->r.min);
    frm->ticked = 0;
    if(frm->p0 > 0)
        frdrawsel(frm, frptofchar(frm, 0), 0, frm->p0, 0);
    if(frm->p1 < frm->nchars)
        frdrawsel(frm, frptofchar(frm, frm->p1), frm->p1, frm->nchars, 0);
    frdrawsel(frm, frptofchar(frm, frm->p0), frm->p0, frm->p1, 1);
    w->lastsr = ZR;
    wscrdraw(w);
}
/*e: function wrefresh */

/*s: function wclose */
bool
wclose(Window *w)
{
    int i;

    i = decref(w);
    if(i > 0)
        return false;

    if(i < 0)
        error("negative ref count");
    if(!w->deleted)
        wclosewin(w);
    wsendctlmesg(w, Exited, ZR, nil);
    return true;
}
/*e: function wclose */






/*s: function wrepaint */
void
wrepaint(Window *w)
{

    /*s: [[wrepaint()]] update cols */
    wsetcols(w);
    /*e: [[wrepaint()]] update cols */
    /*s: [[wrepaint()]] if mouse not opened */
    if(!w->mouseopen)
        frredraw(&w->frm);
    /*e: [[wrepaint()]] if mouse not opened */

    if(w == input){
        wborder(w, Selborder);
        wsetcursor(w, false);
    }else
        wborder(w, Unselborder);
}
/*e: function wrepaint */


/*s: function winborder */
bool
winborder(Window *w, Point xy)
{
    return ptinrect(xy, w->screenr) && 
           !ptinrect(xy, insetrect(w->screenr, Selborder));
}
/*e: function winborder */


/*s: function wmovemouse */
/*
 * Convert back to physical coordinates
 */
void
wmovemouse(Window *w, Point p)
{
    p.x += w->screenr.min.x - w->i->r.min.x;
    p.y += w->screenr.min.y - w->i->r.min.y;
    moveto(mousectl, p);
}
/*e: function wmovemouse */


/*s: function wpointto */
Window*
wpointto(Point pt)
{
    int i;
    Window *v, *w;

    w = nil;
    for(i=0; i<nwindow; i++){
        v = windows[i];
        if(ptinrect(pt, v->screenr))
         if(!v->deleted)
          if(w==nil || v->topped > w->topped)
            w = v;
    }
    return w;
}
/*e: function wpointto */

/*s: function wcurrent */
void
wcurrent(Window *w)
{
    Window *oi;

    /*s: [[wcurrent()]] if wkeyboard */
    if(wkeyboard!=nil && w==wkeyboard)
        return;
    /*e: [[wcurrent()]] if wkeyboard */
    oi = input;
    // updated input!
    input = w;

    if(oi && oi != w)
        wrepaint(oi);
    if(w){
        wrepaint(w);
        wsetcursor(w, false);
    }
    /*s: [[wcurrent()]] wakeup w and oi */
    if(w != oi){
        if(oi){
            oi->wctlready = true;
            wsendctlmesg(oi, Wakeup, ZR, nil);
        }
        if(w){
            w->wctlready = true;
            wsendctlmesg(w, Wakeup, ZR, nil);
        }
    }
    /*e: [[wcurrent()]] wakeup w and oi */
}
/*e: function wcurrent */

/*s: function wsetcursor */
void
wsetcursor(Window *w, bool force)
{
    Cursor *p;

    if(w==nil || w->i==nil || Dx(w->screenr)<=0)
        p = nil;
    else if(wpointto(mouse->xy) == w){
        p = w->cursorp;
        /*s: [[wsetcursor()]] if holding */
        if(p==nil && w->holding)
            p = &whitearrow;
        /*e: [[wsetcursor()]] if holding */
    }else
        p = nil;

    if(!menuing)
        riosetcursor(p, force);
}
/*e: function wsetcursor */


/*s: function wtop */
Window*
wtop(Point pt)
{
    Window *w;

    w = wpointto(pt);
    if(w){
        if(w->topped == topped)
            return nil;
        topwindow(w->i); // draw.h
        wcurrent(w);
        flushimage(display, true);
        w->topped = ++topped;
    }
    return w;
}
/*e: function wtop */

/*s: function wtopme */
void
wtopme(Window *w)
{
    if(w!=nil && w->i!=nil && !w->deleted && w->topped!=topped){
        topwindow(w->i);
        flushimage(display, 1);
        w->topped = ++topped;
    }
}
/*e: function wtopme */

/*s: function wbottomme */
void
wbottomme(Window *w)
{
    if(w!=nil && w->i!=nil && !w->deleted){
        bottomwindow(w->i);
        flushimage(display, 1);
        w->topped = - ++topped;
    }
}
/*e: function wbottomme */

/*s: function wlookid */
Window*
wlookid(int id)
{
    int i;

    for(i=0; i<nwindow; i++)
        if(windows[i]->id == id)
            return windows[i];
    return nil;
}
/*e: function wlookid */

/*s: function wclosewin */
void
wclosewin(Window *w)
{
    int i;

    w->deleted = true;

    if(w == input){
        input = nil;
        wsetcursor(w, false);
    }
    /*s: [[wclosewin()]] if wkeyboard */
    if(w == wkeyboard)
        wkeyboard = nil;
    /*e: [[wclosewin()]] if wkeyboard */
    /*s: [[wclosewin()]] remove w from hidden */
    // delete_list(w, hidden)
    for(i=0; i<nhidden; i++)
        if(hidden[i] == w){
            --nhidden;
            memmove(hidden+i, hidden+i+1, (nhidden-i)*sizeof(hidden[0]));
            hidden[nhidden] = nil;
            break;
        }
    /*e: [[wclosewin()]] remove w from hidden */

    for(i=0; i<nwindow; i++)
        if(windows[i] == w){
            --nwindow;
            memmove(windows+i, windows+i+1, (nwindow-i)*sizeof(Window*));

            freeimage(w->i);
            w->i = nil;
            return;
        }
    error("unknown window in closewin");
}
/*e: function wclosewin */

/*s: function wsetpid */
void
wsetpid(Window *w, int pid, bool dolabel)
{
    char buf[128];
    fdt fd;

    w->pid = pid;

    if(dolabel){
        sprint(buf, "rc %d", pid);
        free(w->label);
        w->label = estrdup(buf);
    }

    sprint(buf, "/proc/%d/notepg", pid);
    fd = open(buf, OWRITE|OCEXEC);
    if(w->notefd > 0)
        close(w->notefd);
    w->notefd = fd;
}
/*e: function wsetpid */


/*e: windows/rio/wind.c */
