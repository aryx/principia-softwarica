/*s: lib_graphics/libdraw/init.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: global display */
// option<Display>
Display	*display;
/*e: global display */
/*s: global font */
Font	*font;
/*e: global font */
/*s: global screen */
// option<Image>
Image	*view;
/*e: global screen */
/*s: global _drawdebug */
bool	_drawdebug = 0;
/*e: global _drawdebug */

/*s: global deffontname */
static char deffontname[] = "*default*";
/*e: global deffontname */
/*s: global _screen */
Screen	*screen;
/*e: global _screen */

/*s: global debuglockdisplay */
bool		debuglockdisplay = false;
/*e: global debuglockdisplay */

static void _closedisplay(Display*, int);

/*s: function drawshutdown */
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
/*e: function drawshutdown */

/*s: function geninitdraw */
errorneg1
geninitdraw(char *devdir, void(*error)(Display*, char*), char *fontname, char *label, char *windir, int ref)
{
    /*s: [[geninitdraw()]] locals */
    int fd, n;
    Subfont *df;
    char buf[128];
    /*e: [[geninitdraw()]] locals */

    display = initdisplay(devdir, windir, error);
    if(display == nil)
        return ERROR_NEG1;

    /*s: [[geninitdraw()]] set up font */
    /*
     * Set up default font
     */

    // Set up default subfont
    df = getdefont(display);
    display->defaultsubfont = df;

    if(df == nil){
        fprint(2, "imageinit: can't open default subfont: %r\n");
    Error:
        closedisplay(display);
        display = nil;
        return ERROR_NEG1;
    }

    // Set up default font
    if(fontname == nil){
        fd = open("/env/font", OREAD);
        if(fd >= 0){
            n = read(fd, buf, sizeof(buf));
            if(n>0 && n<sizeof buf-1){
                buf[n] = 0;
                fontname = buf;
            }
            close(fd);
        }
    }
    /*
     * Build fonts with caches==depth of screen, for speed.
     * If conversion were faster, we'd use 0 and save memory.
     */
    if(fontname == nil){
        snprint(buf, sizeof buf, "%d %d\n0 %d\t%s\n", df->height, df->ascent,
            df->n-1, deffontname);
     //BUG: Need something better for this	installsubfont("*default*", df);
        font = buildfont(display, buf, deffontname);
        if(font == nil){
            fprint(2, "imageinit: can't open default font: %r\n");
            goto Error;
        }
    }else{
        font = openfont(display, fontname);	/* BUG: grey fonts */
        if(font == nil){
            fprint(2, "imageinit: can't open font %s: %r\n", fontname);
            goto Error;
        }
    }
    display->defaultfont = font;
    /*e: [[geninitdraw()]] set up font */
    /*s: [[geninitdraw()]] write label */
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
    /*e: [[geninitdraw()]] write label */
    /*s: [[geninitdraw()]] get window */
    snprint(buf, sizeof buf, "%s/winname", display->windir);
    if(gengetwindow(display, buf, &view, &screen, ref) < 0)
        goto Error;
    /*e: [[geninitdraw()]] get window */

    atexit(drawshutdown);

    return OK_1;
}
/*e: function geninitdraw */

/*s: function initdraw */
errorneg1
initdraw(void(*error)(Display*, char*), char *fontname , char *label)
{
    char *dev = "/dev";

    if(access("/dev/draw/new", AEXIST)<0 && bind("#i", "/dev", MAFTER)<0){
        fprint(2, "imageinit: can't bind /dev/draw: %r\n");
        return ERROR_NEG1;
    }
    return geninitdraw(dev, error, fontname, label, dev, Refnone);
}
/*e: function initdraw */

/*s: function gengetwindow */
/*
 * Attach, or possibly reattach, to window.
 * If reattaching, maintain value of screen pointer.
 */
int
gengetwindow(Display *d, char *winname, Image **winp, Screen **scrp, int ref)
{
    int n, fd;
    char buf[64+1];
    Image *image;
    Rectangle r;

    fd = open(winname, OREAD);
    if(fd<0 || (n=read(fd, buf, sizeof buf-1))<=0){
        if((image=d->image) == nil){
            fprint(2, "gengetwindow: %r\n");
            *winp = nil;
            d->screenimage = nil;
            return -1;
        }
        strcpy(buf, "noborder");
    }else{
        close(fd);
        buf[n] = '\0';
        if(*winp != nil){
            _freeimage1(*winp);
            freeimage((*scrp)->image);
            freescreen(*scrp);
            *scrp = nil;
        }
        image = namedimage(d, buf);
        if(image == 0){
            fprint(2, "namedimage %s failed: %r\n", buf);
            *winp = nil;
            d->screenimage = nil;
            return -1;
        }
        assert(image->chan != 0);
    }

    d->screenimage = image;
    *scrp = allocscreen(image, d->white, 0);
    if(*scrp == nil){
        freeimage(d->screenimage);
        *winp = nil;
        d->screenimage = nil;
        return -1;
    }

    r = image->r;
    if(strncmp(buf, "noborder", 8) != 0)
        r = insetrect(image->r, Borderwidth);
    *winp = _allocwindow(*winp, *scrp, r, ref, DWhite);
    if(*winp == nil){
        freescreen(*scrp);
        *scrp = nil;
        freeimage(image);
        d->screenimage = nil;
        return -1;
    }
    d->screenimage = *winp;
    assert((*winp)->chan != 0);
    return 1;
}
/*e: function gengetwindow */

/*s: function getwindow */
int
getwindow(Display *d, int ref)
{
    char winname[128];

    snprint(winname, sizeof winname, "%s/winname", d->windir);
    return gengetwindow(d, winname, &view, &screen, ref);
}
/*e: function getwindow */

/*s: constant NINFO */
#define	NINFO	12*12
/*e: constant NINFO */

/*s: function initdisplay */
Display*
initdisplay(char *dev, char *win, void(*error)(Display*, char*))
{
    /*s: [[initdisplay()]] locals */
    Display *disp;
    Image *image;

    fdt ctlfd;
    char info[NINFO+1];
    fdt datafd;
    fdt reffd;

    char buf[128];
    char *t;
    bool isnew;
    int n;
    Dir *dir;
    /*e: [[initdisplay()]] locals */

    /*s: [[initdisplay()]] install dumpers */
    fmtinstall('P', Pfmt);
    fmtinstall('R', Rfmt);
    /*e: [[initdisplay()]] install dumpers */
    /*s: [[initdisplay()]] check arguments validity */
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
    /*e: [[initdisplay()]] check arguments validity */

    sprint(buf, "%s/draw/new", dev);
    ctlfd = open(buf, ORDWR|OCEXEC);

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

    n=read(ctlfd, info, sizeof info);

    if(n < 12){
    Error2:
        close(ctlfd);
        goto Error1;
    }
    if(n==NINFO+1)
        n = NINFO;
    info[n] = '\0';

    isnew = false;
    if(n < NINFO)	/* this will do for now, we need something better here */
        isnew = true;

    sprint(buf, "%s/draw/%d/data", dev, atoi(info+0*12));
    datafd = open(buf, ORDWR|OCEXEC);

    if(datafd < 0)
        goto Error2;

    sprint(buf, "%s/draw/%d/refresh", dev, atoi(info+0*12));
    reffd = open(buf, OREAD|OCEXEC);

    if(reffd < 0){
    Error3:
        close(datafd);
        goto Error2;
    }

    // our display!
    disp = mallocz(sizeof(Display), 1);

    if(disp == nil){
    Error4:
        close(reffd);
        goto Error3;
    }

    image = nil;
    if(0){
    Error5:
        free(image);
        free(disp);
        goto Error4;
    }
    if(n >= NINFO){
        image = mallocz(sizeof(Image), 1);
        if(image == nil)
            goto Error5;
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

    disp->_isnewdisplay = isnew;

    disp->bufsize = iounit(datafd);
    if(disp->bufsize <= 0)
        disp->bufsize = 8000;
    if(disp->bufsize < 512){
        werrstr("iounit %d too small", disp->bufsize);
        goto Error5;
    }
    disp->buf = malloc(disp->bufsize+5);	/* +5 for flush message */
    if(disp->buf == nil)
        goto Error5;

    disp->image = image;
    disp->dirno = atoi(info+0*12);
    disp->fd    = datafd;
    disp->ctlfd = ctlfd;
    disp->reffd = reffd;
    disp->bufp = disp->buf;
    disp->error = error;
    disp->windir = t;
    disp->devdir = strdup(dev);

    qlock(&disp->qlock);
    disp->white = allocimage(disp, Rect(0, 0, 1, 1), GREY1, 1, DWhite);
    disp->black = allocimage(disp, Rect(0, 0, 1, 1), GREY1, 1, DBlack);
    if(disp->white == nil || disp->black == nil){
        free(disp->devdir);
        free(disp->white);
        free(disp->black);
        goto Error5;
    }
    disp->opaque = disp->white;
    disp->transparent = disp->black;

    dir = dirfstat(ctlfd);
    if(dir!=nil && dir->type=='i'){
        disp->local = true;
        disp->dataqid = dir->qid.path;
    }
    if(dir!=nil && dir->qid.vers==1)	/* other way to tell */
        disp->_isnewdisplay = true;
    free(dir);

    return disp;
}
/*e: function initdisplay */

/*s: function closedisplay */
/*
 * Call with d unlocked.
 * Note that disp->defaultfont and defaultsubfont are not freed here.
 */
void
closedisplay(Display *disp)
{
    _closedisplay(disp, false);
}
/*e: function closedisplay */

/*s: function _closedisplay */
static void
_closedisplay(Display *disp, bool isshutdown)
{
    int fd;
    char buf[128];

    if(disp == nil)
        return;
    if(disp == display)
        display = nil;

    /*s: [[_closedisplay()]] if oldlabel */
    if(disp->oldlabel[0]){
        snprint(buf, sizeof buf, "%s/label", disp->windir);
        fd = open(buf, OWRITE);
        if(fd >= 0){
            write(fd, disp->oldlabel, strlen(disp->oldlabel));
            close(fd);
        }
    }
    /*e: [[_closedisplay()]] if oldlabel */

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
/*e: function _closedisplay */

/*s: function lockdisplay */
void
lockdisplay(Display *disp)
{
    /*s: [[lockdisplay()]] if debuglockdisplay */
    if(debuglockdisplay){
        /* avoid busy looping; it's rare we collide anyway */
        while(!canqlock(&disp->qlock)){
            fprint(1, "proc %d waiting for display lock...\n", getpid());
            sleep(1000);
        }
    }
    /*e: [[lockdisplay()]] if debuglockdisplay */
    else
        qlock(&disp->qlock);
}
/*e: function lockdisplay */

/*s: function unlockdisplay */
void
unlockdisplay(Display *disp)
{
    qunlock(&disp->qlock);
}
/*e: function unlockdisplay */

/*s: function drawerror */
void
drawerror(Display *d, char *s)
{
    char err[ERRMAX];

    if(d && d->error)
        d->error(d, s);
    else{
        errstr(err, sizeof err);
        fprint(STDERR, "draw: %s: %s\n", s, err);
        exits(s);
    }
}
/*e: function drawerror */

/*s: function doflush */
static
errorneg1
doflush(Display *d)
{
    int n, nn;

    n = d->bufp-d->buf;
    if(n <= 0)
        return OK_1;

    nn=write(d->fd, d->buf, n);
    if(nn != n){
        /*s: [[doflush()]] if _drawdebug */
        if(_drawdebug)
            fprint(2, "flushimage fail: d=%p: n=%d nn=%d %r\n", d, n, nn); /**/
        /*e: [[doflush()]] if _drawdebug */
        d->bufp = d->buf;	/* might as well; chance of continuing */
        return ERROR_NEG1;
    }
    d->bufp = d->buf;
    return OK_1;
}
/*e: function doflush */

/*s: function flushimage */
int
flushimage(Display *d, bool visible)
{
    if(d == nil)
        return 0;
    if(visible){
        *d->bufp++ = 'v';	/* five bytes always reserved for this */
        if(d->_isnewdisplay){
            BPLONG(d->bufp, d->screenimage->id);
            d->bufp += 4;
        }
    }
    return doflush(d);
}
/*e: function flushimage */

/*s: function bufimage */
byte*
bufimage(Display *d, int n)
{
    uchar *p;

    if(n < 0 || n > d->bufsize){
        werrstr("bad count in bufimage");
        return nil;
    }
    if(d->bufp + n  >  d->buf + d->bufsize)
        if(doflush(d) < 0)
            return nil;
    p = d->bufp;
    d->bufp += n;
    return p;
}
/*e: function bufimage */

/*e: lib_graphics/libdraw/init.c */
