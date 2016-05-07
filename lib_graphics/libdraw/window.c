/*s: lib_graphics/libdraw/window.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <window.h>
#include <draw_private.h>

// see getwindow()
/*s: global screen */
Screen	*screen;
/*e: global screen */

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

/*s: function allocwindow */
Image*
allocwindow(Screen *s, Rectangle r, int ref, rgba val)
{
    return _allocwindow(nil, s, r, ref, val);
}
/*e: function allocwindow */



/*s: function gengetwindow */
/*
 * Attach, or possibly reattach, to window.
 * If reattaching, maintain value of screen pointer.
 */
errorneg1
gengetwindow(Display *d, char *winname, Image **winp, Screen **scrp, int ref)
{
    int n;
    fdt fd;
    char buf[64+1]; // /dev/winname content
    Image *image;
    Rectangle r;

    fd = open(winname, OREAD);
    if(fd<0 || (n=read(fd, buf, sizeof buf-1))<=0){
        // no /dev/winname, image is then the full screen
        image = d->image;
        /*s: [[gengetwindow()]] sanity check image from display */
        if(image == nil){
            fprint(2, "gengetwindow: %r\n");
            /*s: [[gengetwindow()]] return error */
            *winp = nil;
            d->screenimage = nil;
            return ERROR_NEG1;
            /*e: [[gengetwindow()]] return error */
        }
        /*e: [[gengetwindow()]] sanity check image from display */
        strcpy(buf, "noborder");
    }else{
        close(fd);
        buf[n] = '\0';
        /*s: [[gengetwindow()]] if already had a view, free it */
        if(*winp != nil){
            _freeimage1(*winp);
            freeimage((*scrp)->image);
            freescreen(*scrp);
            *scrp = nil;
        }
        /*e: [[gengetwindow()]] if already had a view, free it */
        // get our window!
        image = namedimage(d, buf);
        /*s: [[gengetwindow()]] sanity check image from namedimage */
        if(image == nil){
            fprint(2, "namedimage %s failed: %r\n", buf);
            /*s: [[gengetwindow()]] return error */
            *winp = nil;
            d->screenimage = nil;
            return ERROR_NEG1;
            /*e: [[gengetwindow()]] return error */
        }
        assert(image->chan != 0);
        /*e: [[gengetwindow()]] sanity check image from namedimage */
    }

    d->screenimage = image;
    *scrp = allocscreen(image, d->white, false);
    /*s: [[gengetwindow()]] sanity check srcp */
    if(*scrp == nil){
        freeimage(d->screenimage);
        /*s: [[gengetwindow()]] return error */
        *winp = nil;
        d->screenimage = nil;
        return ERROR_NEG1;
        /*e: [[gengetwindow()]] return error */
    }
    /*e: [[gengetwindow()]] sanity check srcp */

    r = image->r;
    if(strncmp(buf, "noborder", 8) != 0)
        r = insetrect(image->r, Borderwidth);
    *winp = _allocwindow(*winp, *scrp, r, ref, DWhite);
    /*s: [[gengetwindow()]] sanity check winp */
    if(*winp == nil){
        freescreen(*scrp);
        *scrp = nil;
        freeimage(image);
        /*s: [[gengetwindow()]] return error */
        *winp = nil;
        d->screenimage = nil;
        return ERROR_NEG1;
        /*e: [[gengetwindow()]] return error */
    }
    /*e: [[gengetwindow()]] sanity check winp */
    d->screenimage = *winp;
    assert((*winp)->chan != 0);
    return OK_1;
}
/*e: function gengetwindow */

/*s: function getwindow */
errorneg1
getwindow(Display *d, int ref)
{
    char winname[128];

    snprint(winname, sizeof winname, "%s/winname", d->windir);
    return gengetwindow(d, winname, &view, &screen, ref);
}
/*e: function getwindow */




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


/*s: function newwindow */
/* Connect us to new window, if possible */
errorneg1
newwindow(char *str)
{
    fdt fd;
    char *wsys;
    char buf[256];

    wsys = getenv("wsys");
    /*s: [[newwindow()]] sanity check wsys */
    if(wsys == nil)
        return ERROR_NEG1;
    /*e: [[newwindow()]] sanity check wsys */
    fd = open(wsys, ORDWR);
    free(wsys);
    /*s: [[newwindow()]] sanity check fd */
    if(fd < 0)
        return ERROR_NEG1;
    /*e: [[newwindow()]] sanity check fd */
    rfork(RFNAMEG);

    if(str)
        snprint(buf, sizeof buf, "new %s", str);
    else
        strcpy(buf, "new");

    return mount(fd, -1, "/dev", MBEFORE, buf);
}
/*e: function newwindow */
/*e: lib_graphics/libdraw/window.c */
