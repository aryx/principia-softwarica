/*s: lib_graphics/libdraw/init.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>
#include <window.h>
#include <font.h>

/*s: global [[deffontname]] */
static char deffontname[] = "*default*";
/*e: global [[deffontname]] */

static void _closedisplay(Display*, int);

/*s: function [[drawshutdown]] */
/* note handler */
static void
drawshutdown(void)
{
    Display *d;

    d = display;
    if(d){
        display = nil;
        _closedisplay(d, true);
    }
}
/*e: function [[drawshutdown]] */

/*s: function [[geninitdraw]] */
errorneg1
geninitdraw(char *devdir, Errorfn error, char *fontname, char *label, char *windir, int ref)
{
    /*s: [[geninitdraw()]] locals */
    Subfont *df;
    /*x: [[geninitdraw()]] locals */
    fdt fd;
    int n;
    char buf[128];
    /*e: [[geninitdraw()]] locals */

    display = initdisplay(devdir, windir, error);
    /*s: [[geninitdraw()]] sanity check display */
    if(display == nil)
        return ERROR_NEG1;
    /*e: [[geninitdraw()]] sanity check display */

    /*s: [[geninitdraw()]] set up font */
    /*
     * Set up default font
     */

    // Set up default subfont
    df = getdefont(display);
    display->defaultsubfont = df;
    /*s: [[geninitdraw()]] sanity check df */
    if(df == nil){
        fprint(2, "imageinit: can't open default subfont: %r\n");
    Error:
        closedisplay(display);
        display = nil;
        return ERROR_NEG1;
    }
    /*e: [[geninitdraw()]] sanity check df */

    // Set up default font
    /*s: [[geninitdraw()]] read fontname if fontname was nil */
    if(fontname == nil){
        fd = open("/env/font", OREAD);
        if(fd >= 0){
            n = read(fd, buf, sizeof(buf));
            if(n>0 && n < sizeof buf-1){
                buf[n] = '\0';
                fontname = buf;
            }
            close(fd);
        }
    }
    /*e: [[geninitdraw()]] read fontname if fontname was nil */
    /*s: [[geninitdraw()]] if fontname still nil */
    /*
     * Build fonts with caches==depth of screen, for speed.
     * If conversion were faster, we'd use 0 and save memory.
     */
    if(fontname == nil){
        snprint(buf, sizeof buf, "%d %d\n0 %d\t%s\n", 
            df->height, df->ascent,
            df->n-1, deffontname);
        //BUG: Need something better for this	installsubfont("*default*", df);
        font = buildfont(display, buf, deffontname);
        /*s: [[geninitdraw()]] sanity check font part2 */
        if(font == nil){
            fprint(2, "imageinit: can't open default font: %r\n");
            goto Error;
        }
        /*e: [[geninitdraw()]] sanity check font part2 */
    }
    /*e: [[geninitdraw()]] if fontname still nil */
    else{
        font = openfont(display, fontname);	/* BUG: grey fonts */
        /*s: [[geninitdraw()]] sanity check font */
        if(font == nil){
            fprint(2, "imageinit: can't open font %s: %r\n", fontname);
            goto Error;
        }
        /*e: [[geninitdraw()]] sanity check font */
    }
    display->defaultfont = font;
    /*e: [[geninitdraw()]] set up font */
    /*s: [[geninitdraw()]] write new label */
    /*
     * Write label; ignore errors (we might not be running under rio)
     */
    if(label){
        snprint(buf, sizeof buf, "%s/label", display->windir);
        fd = open(buf, OREAD);
        if(fd >= 0){
            read(fd, display->oldlabel, (sizeof display->oldlabel)-1);
            close(fd);
            fd = create(buf, OWRITE, 0666);
            if(fd >= 0){
                write(fd, label, strlen(label));
                close(fd);
            }
        }
    }
    /*e: [[geninitdraw()]] write new label */
    /*s: [[geninitdraw()]] get window */
    snprint(buf, sizeof buf, "%s/winname", display->windir);
    if(gengetwindow(display, buf, &view, &screen, ref) < 0)
        goto Error;
    /*e: [[geninitdraw()]] get window */
    atexit(drawshutdown);

    return OK_1;
}
/*e: function [[geninitdraw]] */

/*s: function [[initdraw]] */
errorneg1
initdraw(Errorfn error, char *fontname , char *label)
{

    if(access("/dev/draw/new", AEXIST)<0 && bind("#i", "/dev", MAFTER)<0){
        fprint(2, "imageinit: can't bind /dev/draw: %r\n");
        return ERROR_NEG1;
    }
    return geninitdraw("/dev", error, fontname, label, "/dev", Refnone);
}
/*e: function [[initdraw]] */

/*s: constant [[NINFO]] */
#define	NINFO	12*12
/*e: constant [[NINFO]] */

/*s: function [[initdisplay]] */
Display*
initdisplay(char *dev, char *win, Errorfn error)
{
    /*s: [[initdisplay()]] locals */
    fdt ctlfd;
    fdt datafd;
    fdt reffd;

    char info[NINFO+1];
    char buf[128];

    Display *disp;
    Image *image;

    int n;
    /*x: [[initdisplay()]] locals */
    char *t;
    /*e: [[initdisplay()]] locals */

    /*s: [[initdisplay()]] install dumpers */
    fmtinstall('P', Pfmt);
    fmtinstall('R', Rfmt);
    /*e: [[initdisplay()]] install dumpers */
    /*s: [[initdisplay()]] sanity check arguments */
    if(dev == nil)
        dev = "/dev";
    if(win == nil)
        win = "/dev";

    if(strlen(dev)>sizeof buf-25 || strlen(win)>sizeof buf-25){
        werrstr("initdisplay: directory name too long");
        return nil;
    }
    t = strdup(win);
    if(t == nil)
        return nil;
    /*e: [[initdisplay()]] sanity check arguments */

    sprint(buf, "%s/draw/new", dev);
    ctlfd = open(buf, ORDWR|OCEXEC);
    /*s: [[initdisplay()]] sanity check ctlfd */
    if(ctlfd < 0){
        if(bind("#i", dev, MAFTER) < 0){
    Error1:
            free(t);
            werrstr("initdisplay: %s: %r", buf);
            return nil;
        }
        // try again
        ctlfd = open(buf, ORDWR|OCEXEC);
    }
    if(ctlfd < 0)
        goto Error1;
    /*e: [[initdisplay()]] sanity check ctlfd */

    n=read(ctlfd, info, sizeof info);
    /*s: [[initdisplay()]] sanity check read ctlfd */
    if(n < 12){
    Error2:
        close(ctlfd);
        goto Error1;
    }
    if(n==NINFO+1)
        n = NINFO;
    /*e: [[initdisplay()]] sanity check read ctlfd */
    info[n] = '\0';

    sprint(buf, "%s/draw/%d/data", dev, atoi(info+0*12));
    datafd = open(buf, ORDWR|OCEXEC);
    /*s: [[initdisplay()]] sanity check datafd */
    if(datafd < 0)
        goto Error2;
    /*e: [[initdisplay()]] sanity check datafd */

    sprint(buf, "%s/draw/%d/refresh", dev, atoi(info+0*12));
    reffd = open(buf, OREAD|OCEXEC);
    /*s: [[initdisplay()]] sanity check reffd */
    if(reffd < 0){
    Error3:
        close(datafd);
        goto Error2;
    }
    /*e: [[initdisplay()]] sanity check reffd */

    // our display!
    disp = mallocz(sizeof(Display), true);
    /*s: [[initdisplay()]] sanity check disp */
    if(disp == nil){
    Error4:
        close(reffd);
        goto Error3;
    }
    /*e: [[initdisplay()]] sanity check disp */
    disp->dirno = atoi(info+0*12);

    image = nil;
    /*s: [[initdisplay()]] sanity check image part1 */
    if(0){
    Error5:
        free(image);
        free(disp);
        goto Error4;
    }
    /*e: [[initdisplay()]] sanity check image part1 */
    if(n >= NINFO){
        image = mallocz(sizeof(Image), true);
        /*s: [[initdisplay()]] sanity check image part2 */
        if(image == nil)
            goto Error5;
        /*e: [[initdisplay()]] sanity check image part2 */

        image->display = disp;
        image->id = 0; // info+1*12 but should always be 0
        image->chan = strtochan(info+2*12);
        image->depth = chantodepth(image->chan);
        image->repl = atoi(info+3*12);

        image->r.min.x = atoi(info+4*12);
        image->r.min.y = atoi(info+5*12);
        image->r.max.x = atoi(info+6*12);
        image->r.max.y = atoi(info+7*12);

        image->clipr.min.x = atoi(info+8*12);
        image->clipr.min.y = atoi(info+9*12);
        image->clipr.max.x = atoi(info+10*12);
        image->clipr.max.y = atoi(info+11*12);
    }
    disp->image = image;
    disp->fd    = datafd;
    disp->ctlfd = ctlfd;
    disp->reffd = reffd;

    /*s: [[initdisplay()]] set display bufsize */
    disp->bufsize = iounit(datafd);
    if(disp->bufsize <= 0)
        disp->bufsize = 8000;
    /*s: [[initdisplay()]] sanity check bufsize */
    if(disp->bufsize < 512){
        werrstr("iounit %d too small", disp->bufsize);
        goto Error5;
    }
    /*e: [[initdisplay()]] sanity check bufsize */
    /*e: [[initdisplay()]] set display bufsize */
    disp->buf = malloc(disp->bufsize+5);	/* +5 for flush message */
    /*s: [[initdisplay()]] sanity check buf */
    if(disp->buf == nil)
        goto Error5;
    /*e: [[initdisplay()]] sanity check buf */
    disp->bufp = disp->buf;

    disp->error = error;
    disp->windir = t;
    disp->devdir = strdup(dev);

    qlock(&disp->qlock); // released in closedisplay (why not earlier? first API call?)
  
    // first API calls!
    disp->white = allocimage(disp, Rect(0, 0, 1, 1), GREY1, true, DWhite);
    disp->black = allocimage(disp, Rect(0, 0, 1, 1), GREY1, true, DBlack);
    /*s: [[initdisplay()]] sanity check white and black */
    if(disp->white == nil || disp->black == nil){
        free(disp->devdir);
        free(disp->white);
        free(disp->black);
        goto Error5;
    }
    /*e: [[initdisplay()]] sanity check white and black */
    disp->opaque = disp->white;
    disp->transparent = disp->black;

    return disp;
}
/*e: function [[initdisplay]] */

/*s: function [[closedisplay]] */
/*
 * Call with d unlocked.
 * Note that disp->defaultfont and defaultsubfont are not freed here.
 */
void
closedisplay(Display *disp)
{
    _closedisplay(disp, false);
}
/*e: function [[closedisplay]] */

/*s: function [[_closedisplay]] */
static void
_closedisplay(Display *disp, bool isshutdown)
{
    fdt fd;
    char buf[128];

    /*s: [[_closedisplay()]] sanity check disp */
    if(disp == nil)
        return;
    /*e: [[_closedisplay()]] sanity check disp */
    if(disp == display)
        display = nil;

    /*s: [[_closedisplay()]] restore oldlabel */
    if(disp->oldlabel[0]){
        snprint(buf, sizeof buf, "%s/label", disp->windir);
        fd = open(buf, OWRITE);
        if(fd >= 0){
            write(fd, disp->oldlabel, strlen(disp->oldlabel));
            close(fd);
        }
    }
    /*e: [[_closedisplay()]] restore oldlabel */

    /*
     * if we're shutting down, don't free all the resources.
     * if other procs are getting shot down by notes too,
     * one might get shot down while holding the malloc lock.
     * just let the kernel clean things up when we exit.
     */
    if(isshutdown)
        return;

    free(disp->devdir);
    free(disp->windir);
    freeimage(disp->white);
    freeimage(disp->black);
    close(disp->fd);
    close(disp->ctlfd);
    /* should cause refresh slave to shut down */
    close(disp->reffd);

    qunlock(&disp->qlock);
    free(disp);
}
/*e: function [[_closedisplay]] */

/*e: lib_graphics/libdraw/init.c */
