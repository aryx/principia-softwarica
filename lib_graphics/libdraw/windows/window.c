/*s: lib_graphics/libdraw/window.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

// dup from memdraw.h
typedef struct Memimage Memimage;

/*s: global screenid */
static int	screenid;
/*e: global screenid */

/*s: function allocscreen */
Screen*
allocscreen(Image *image, Image *fill, bool public)
{
    Display *d;
    Screen *s;
    int id, try;
    byte *a;

    d = image->display;
    /*s: [[allocscreen()]] sanity check images have same display */
    if(d != fill->display){
        werrstr("allocscreen: image and fill on different displays");
        return nil;
    }
    /*e: [[allocscreen()]] sanity check images have same display */
    s = malloc(sizeof(Screen));
    /*s: [[allocscreen()]] sanity check s */
    if(s == nil)
        return nil;
    /*e: [[allocscreen()]] sanity check s */
    SET(id);
    for(try=0; try<25; try++){
        /* loop until find a free id */
        /*s: [[allocscreen()]] marshall allocate screen message */
        // allocate screen: 'A' id[4] imageid[4] fillid[4] public[1]
        a = bufimage(d, 1+4+4+4+1);
        /*s: [[allocscreen()]] sanity check a */
        if(a == nil){
            free(s);
            return nil;
        }
        /*e: [[allocscreen()]] sanity check a */
        id = ++screenid;
        a[0] = 'A';
        BPLONG(a+1, id);
        BPLONG(a+5, image->id);
        BPLONG(a+9, fill->id);
        a[13] = public;
        if(flushimage(d, false) != -1)
            break;
        /*e: [[allocscreen()]] marshall allocate screen message */
    }
    s->display = d;
    s->id = id;
    s->image = image;
    s->fill = fill;
    /*s: [[allocscreen()]] sanity check screen image */
    assert(s->image && s->image->chan != 0);
    /*e: [[allocscreen()]] sanity check screen image */
    return s;
}
/*e: function allocscreen */

/*s: function publicscreen */
Screen*
publicscreen(Display *d, int id, ulong chan)
{
    Screen *s;
    byte *a;

    s = malloc(sizeof(Screen));
    /*s: [[publicscreen()]] sanity check s */
    if(s == nil)
        return nil;
    /*e: [[publicscreen()]] sanity check s */

    /*s: [[publicscreen()]] marshall use public screen message */
    // use public screen: 'S' id[4] chan[4]
    a = bufimage(d, 1+4+4);
    /*s: [[publicscreen()]] sanity check a */
    if(a == nil){
    Error:
        free(s);
        return nil;
    }
    /*e: [[publicscreen()]] sanity check a */
    a[0] = 'S';
    BPLONG(a+1, id);
    BPLONG(a+5, chan);
    if(flushimage(d, false) < 0)
        goto Error;
    /*e: [[publicscreen()]] marshall use public screen message */

    s->display = d;
    s->id = id;

    s->image = nil;
    s->fill = nil;

    return s;
}
/*e: function publicscreen */

/*s: function freescreen */
errorneg1
freescreen(Screen *s)
{
    Display *d;
    byte *a;

    /*s: [[freescreen()]] sanity check s */
    if(s == nil)
        return 0;
    /*e: [[freescreen()]] sanity check s */
    d = s->display;
    /*s: [[freescreen()]] marshall free screen message */
    // free screen: 'F' id[4]
    a = bufimage(d, 1+4);
    /*s: [[freescreen()]] sanity check a */
    if(a == nil)
        return ERROR_NEG1;
    /*e: [[freescreen()]] sanity check a */
    a[0] = 'F';
    BPLONG(a+1, s->id);
    /*
     * flush(true) because screen is likely holding last reference to
     * window, and want it to disappear visually.
     */
    if(flushimage(d, true) < 0)
        return ERROR_NEG1;
    /*e: [[freescreen()]] marshall free screen message */
    free(s);
    return OK_1;
}
/*e: function freescreen */

/*s: function allocwindow */
Image*
allocwindow(Screen *s, Rectangle r, int ref, rgba val)
{
    return _allocwindow(nil, s, r, ref, val);
}
/*e: function allocwindow */

/*s: function _allocwindow */
Image*
_allocwindow(Image *i, Screen *s, Rectangle r, int ref, rgba val)
{
    Display *d;

    d = s->display;
    i = _allocimage(i, d, r, d->screenimage->chan, false, val, s->id, ref);
    /*s: [[_allocwindow()]] sanity check i */
    if(i == nil)
        return nil;
    /*e: [[_allocwindow()]] sanity check i */
    i->screen = s;

    // add_list(i, display->windows)
    i->next = s->display->windows;
    s->display->windows = i;

    return i;
}
/*e: function _allocwindow */

/*s: function topbottom */
static
void
topbottom(Image **w, int n, bool top)
{
    int i;
    byte *b;
    Display *d;

    /*s: [[topbottom()]] sanity check n */
    if(n < 0){
    Ridiculous:
        fprint(2, "top/bottom: ridiculous number of windows\n");
        return;
    }
    if(n == 0)
        return;
    if(n > (w[0]->display->bufsize - 100)/4)
        goto Ridiculous;
    /*e: [[topbottom()]] sanity check n */
    /*s: [[topbottom()]] sanity check images have same display */
    /*
     * this used to check that all images were on the same screen.
     * we don't know the screen associated with images we acquired
     * by name.  instead, check that all images are on the same display.
     * the display will check that they are all on the same screen.
     */
    d = w[0]->display;
    for(i=1; i<n; i++)
        if(w[i]->display != d){
            fprint(2, "top/bottom: windows not on same screen\n");
            return;
        }
    /*e: [[topbottom()]] sanity check images have same display */

    // top or bottom windows: 't' top[1] nw[2] n*id[4]
    b = bufimage(d, 1+1+2+4*n);
    b[0] = 't';
    b[1] = top;
    BPSHORT(b+2, n);
    for(i=0; i<n; i++)
        BPLONG(b+4+4*i, w[i]->id);
}
/*e: function topbottom */

/*s: function bottomwindow */
void
bottomwindow(Image *w)
{
    /*s: [[xxxmwindow()]] sanity check window w */
    if(w->screen == nil)
        return;
    /*e: [[xxxmwindow()]] sanity check window w */
    topbottom(&w, 1, false);
}
/*e: function bottomwindow */

/*s: function topwindow */
void
topwindow(Image *w)
{
    /*s: [[xxxmwindow()]] sanity check window w */
    if(w->screen == nil)
        return;
    /*e: [[xxxmwindow()]] sanity check window w */
    topbottom(&w, 1, true);
}
/*e: function topwindow */

/*s: function bottomnwindows */
void
bottomnwindows(Image **w, int n)
{
    topbottom(w, n, false);
}
/*e: function bottomnwindows */

/*s: function topnwindows */
void
topnwindows(Image **w, int n)
{
    topbottom(w, n, true);
}
/*e: function topnwindows */

/*s: function originwindow */
errorneg1
originwindow(Image *w, Point log, Point scr)
{
    byte *b;
    Point delta;

    flushimage(w->display, false);

    /*s: [[originwindow()]] marshall position window message */
    // position window: 'o' id[4] r.min [2*4] screenr.min [2*4]
    b = bufimage(w->display, 1+4+2*4+2*4);
    /*s: [[originwindow()]] sanity check b */
    if(b == nil)
        return 0; // really? not -1?
    /*e: [[originwindow()]] sanity check b */
    b[0] = 'o';
    BPLONG(b+1, w->id);
    BPLONG(b+5, log.x);
    BPLONG(b+9, log.y);
    BPLONG(b+13, scr.x);
    BPLONG(b+17, scr.y);
    if(flushimage(w->display, true) < 0)
        return ERROR_NEG1;
    /*e: [[originwindow()]] marshall position window message */

    delta = subpt(log, w->r.min);
    // new image coords
    w->r     = rectaddpt(w->r, delta);
    w->clipr = rectaddpt(w->clipr, delta);
    return OK_1;
}
/*e: function originwindow */
/*e: lib_graphics/libdraw/window.c */
