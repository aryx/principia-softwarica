/*s: windows/rio/rio.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include <cursor.h>
#include <mouse.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>
#include <plumb.h>

#include "dat.h"
#include "fns.h"

/*
 *  WASHINGTON (AP) - The Food and Drug Administration warned
 * consumers Wednesday not to use ``Rio'' hair relaxer products
 * because they may cause severe hair loss or turn hair green....
 *    The FDA urged consumers who have experienced problems with Rio
 * to notify their local FDA office, local health department or the
 * company at 1‑800‑543‑3002.
 */

void	killprocs(void);
int	shutdown(void*, char*);
void	button3menu(void);
void	button2menu(Window*);

void	resize(void);
void	move(void);
void	delete(void);
void	hide(void);
void	unhide(int);
Image	*sweep(void);
Image	*bandsize(Window*);
Image*	drag(Window*, Rectangle*);
void	resized(void);

void	mousethread(void*);
void	keyboardthread(void*);
void 	winclosethread(void*);
void 	deletethread(void*);
void	initcmd(void*);

/*s: global exitchan */
// chan<unit> (listener = threadmain, sender = mousethread(Exit) | ?)
Channel	*exitchan;	/* chan(int) */
/*e: global exitchan */
/*s: global viewr */
Rectangle	viewr;
/*e: global viewr */
/*s: global fontname */
char		*fontname;
/*e: global fontname */

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

/*s: enum _anon_ (windows/rio/rio.c)2 */
enum
{
    Cut,
    Paste,
    Snarf,
    Plumb,
    Send,
    Scroll,
};
/*e: enum _anon_ (windows/rio/rio.c)2 */

/*s: global menu2str */
char*		menu2str[] = {
 [Cut]		"cut",
 [Paste]	"paste",
 [Snarf]	"snarf",
 [Plumb]	"plumb",
 [Send]		"send",
 [Scroll]	"scroll",
 nil
};
/*e: global menu2str */

/*s: global menu2 */
Menu menu2 =
{
    menu2str
};
/*e: global menu2 */

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

/*s: global rcargv */
char *rcargv[] = { "rc", "-i", nil };
/*e: global rcargv */
/*s: global kbdargv */
char *kbdargv[] = { "rc", "-c", nil, nil };
/*e: global kbdargv */



/*s: function derror */
void
derror(Display*, char *errorstr)
{
    error(errorstr);
}
/*e: function derror */

/*s: function usage */
void
usage(void)
{
    fprint(STDERR, "usage: rio [-f font] [-i initcmd] [-k kbdcmd] [-s]\n");
    exits("usage");
}
/*e: function usage */

/*s: function threadmain */
void threadmain(int argc, char *argv[])
{
    /*s: [[main()]] locals */
    char buf[256];
    /*x: [[main()]] locals */
    char *s;
    /*x: [[main()]] locals */
    char *initstr = nil;
    /*x: [[main()]] locals */
    char *kbdin = nil;
    /*x: [[main()]] locals */
    Image *i;
    Rectangle r;
    /*e: [[main()]] locals */

    ARGBEGIN{
    /*s: [[main()]] command line processing */
    case 's':
        scrolling = true;
        break;
    /*x: [[main()]] command line processing */
    case 'i':
        initstr = ARGF();
        if(initstr == nil)
            usage();
        break;
    /*x: [[main()]] command line processing */
    case 'k':
        if(kbdin != nil)
            usage();
        kbdin = ARGF();
        if(kbdin == nil)
            usage();
        break;
    /*x: [[main()]] command line processing */
    case 'f':
        fontname = ARGF();
        if(fontname == nil)
            usage();
        break;
    /*e: [[main()]] command line processing */
    }ARGEND

    /*s: [[main()]] set some globals */
    if(getwd(buf, sizeof buf) == nil)
        startdir = estrdup(".");
    else
        startdir = estrdup(buf);
    /*x: [[main()]] set some globals */
    s = getenv("tabstop");
    if(s != nil)
        maxtab = strtol(s, nil, 0);
    if(maxtab == 0)
        maxtab = 4;
    free(s);
    /*x: [[main()]] set some globals */
    snarffd = open("/dev/snarf", OREAD|OCEXEC);
    /*x: [[main()]] set some globals */
    if(fontname == nil)
        fontname = getenv("font");
    if(fontname == nil)
        fontname = "/lib/font/bit/lucm/unicode.9.font";

    /* check font before barging ahead */
    if(access(fontname, 0) < 0){
        fprint(STDERR, "rio: can't access %s: %r\n", fontname);
        exits("font open");
    }

    putenv("font", fontname);
    /*e: [[main()]] set some globals */

    // Rio, a graphical app

    /*s: [[main()]] graphics initializations */
    if(geninitdraw(nil, derror, nil, "rio", nil, Refnone) < 0){
        fprint(STDERR, "rio: can't open display: %r\n");
        exits("display open");
    }
    viewr = view->r;

    iconinit(); // allocate background and red images

    /*s: [[main()]] mouse initialisation */
    mousectl = initmouse(nil, view);
    if(mousectl == nil)
        error("can't find mouse");
    mouse = mousectl;
    /*e: [[main()]] mouse initialisation */
    /*s: [[main()]] keyboard initialisation */
    keyboardctl = initkeyboard(nil);
    if(keyboardctl == nil)
        error("can't find keyboard");
    /*e: [[main()]] keyboard initialisation */

    wscreen = allocscreen(view, background, false);
    /*s: [[main()]] sanity check wscreen */
    if(wscreen == nil)
        error("can't allocate screen");
    /*e: [[main()]] sanity check wscreen */

    draw(view, viewr, background, nil, ZP);
    flushimage(display, true);
    /*e: [[main()]] graphics initializations */

    // Rio, a concurrent app

    /*s: [[main()]] communication channels creation */
    exitchan     = chancreate(sizeof(int), 0);
    /*x: [[main()]] communication channels creation */
    deletechan   = chancreate(sizeof(char*), 0);
    /*x: [[main()]] communication channels creation */
    winclosechan = chancreate(sizeof(Window*), 0);
    /*e: [[main()]] communication channels creation */
    /*s: [[main()]] threads creation */
    timerinit();

    threadcreate(keyboardthread, nil, STACK);
    threadcreate(mousethread, nil, STACK);
    /*x: [[main()]] threads creation */
    threadcreate(deletethread, nil, STACK);
    /*x: [[main()]] threads creation */
    threadcreate(winclosethread, nil, STACK);
    /*e: [[main()]] threads creation */

    // Rio, a filesystem server

    filsys = filsysinit(xfidinit());
    /*s: [[main()]] if filsys is nil */
    if(filsys == nil)
        fprint(STDERR, "rio: can't create file system server: %r\n");
    /*e: [[main()]] if filsys is nil */
    else{
        /*s: [[main()]] error management after everything setup */
        errorshouldabort = true;/* suicide if there's trouble after this */
        /*s: [[main()]] if initstr or kdbin */
        if(initstr)
            proccreate(initcmd, initstr, STACK);
        /*x: [[main()]] if initstr or kdbin */
        if(kbdin){
            kbdargv[2] = kbdin;
            r = view->r;
            r.max.x = r.min.x+300;
            r.max.y = r.min.y+80;
            i = allocwindow(wscreen, r, Refbackup, DWhite);
            wkeyboard = new(i, false, scrolling, 0, nil, "/bin/rc", kbdargv);
            if(wkeyboard == nil)
                error("can't create keyboard window");
        }
        /*e: [[main()]] if initstr or kdbin */
        threadnotify(shutdown, true);
        /*e: [[main()]] error management after everything setup */

        // blocks until get exit message on exitchan
        recv(exitchan, nil);
    }
    killprocs();
    threadexitsall(nil);
}
/*e: function threadmain */

/*s: function putsnarf */
/*
 * /dev/snarf updates when the file is closed, so we must open our own
 * fd here rather than use snarffd
 */
void
putsnarf(void)
{
    int fd, i, n;

    if(snarffd<0 || nsnarf==0)
        return;
    fd = open("/dev/snarf", OWRITE);
    if(fd < 0)
        return;
    /* snarf buffer could be huge, so fprint will truncate; do it in blocks */
    for(i=0; i<nsnarf; i+=n){
        n = nsnarf-i;
        if(n >= 256)
            n = 256;
        if(fprint(fd, "%.*S", n, snarf+i) < 0)
            break;
    }
    close(fd);
}
/*e: function putsnarf */

/*s: function getsnarf */
void
getsnarf(void)
{
    int i, n, nb, nulls;
    char *sn, buf[1024];

    if(snarffd < 0)
        return;
    sn = nil;
    i = 0;
    seek(snarffd, 0, 0);
    while((n = read(snarffd, buf, sizeof buf)) > 0){
        sn = erealloc(sn, i+n+1);
        memmove(sn+i, buf, n);
        i += n;
        sn[i] = 0;
    }
    if(i > 0){
        snarf = runerealloc(snarf, i+1);
        cvttorunes(sn, i, snarf, &nb, &nsnarf, &nulls);
        free(sn);
    }
}
/*e: function getsnarf */

/*s: function initcmd */
void
initcmd(void *arg)
{
    char *cmd;

    cmd = arg;
    rfork(RFENVG|RFFDG|RFNOTEG|RFNAMEG);
    procexecl(nil, "/bin/rc", "rc", "-c", cmd, nil);
    fprint(STDERR, "rio: exec failed: %r\n");
    exits("exec");
}
/*e: function initcmd */

/*s: global oknotes */
char *oknotes[] =
{
    "delete",
    "hangup",
    "kill",
    "exit",
    nil
};
/*e: global oknotes */

/*s: function shutdown */
int
shutdown(void *, char *msg)
{
    int i;
    static Lock shutdownlk;
    
    killprocs();
    for(i=0; oknotes[i]; i++)
        if(strncmp(oknotes[i], msg, strlen(oknotes[i])) == 0){
            lock(&shutdownlk);	/* only one can threadexitsall */
            threadexitsall(msg);
        }
    fprint(STDERR, "rio %d: abort: %s\n", getpid(), msg);
    abort();
    exits(msg);
    return 0;
}
/*e: function shutdown */

/*s: function killprocs */
void
killprocs(void)
{
    int i;

    for(i=0; i<nwindow; i++)
        postnote(PNGROUP, windows[i]->pid, "hangup");
}
/*e: function killprocs */

/*s: function keyboardthread */
void
keyboardthread(void*)
{
    Rune buf[2][20];
    // points to buf[0] or buf[1]
    Rune *rp;
    int n, i;

    threadsetname("keyboardthread");

    n = 0;
    for(;;){
        rp = buf[n];
        n = 1-n;

        // Listen
        recv(keyboardctl->c, rp);

        for(i=1; i<nelem(buf[0])-1; i++)
            if(nbrecv(keyboardctl->c, rp+i) <= 0)
                break;
        rp[i] = L'\0';

        if(input != nil)
            // Dispatch, to current window thread!
            sendp(input->ck, rp);
    }
}
/*e: function keyboardthread */

/*s: function keyboardsend */
/*
 * Used by /dev/kbdin
 */
void
keyboardsend(char *s, int cnt)
{
    Rune *r;
    int i, nb, nr;

    r = runemalloc(cnt);
    /* BUGlet: partial runes will be converted to error runes */
    cvttorunes(s, cnt, r, &nb, &nr, nil);
    for(i=0; i<nr; i++)
        send(keyboardctl->c, &r[i]);
    free(r);
}
/*e: function keyboardsend */

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

/*s: function winclosethread */
/* thread to allow fsysproc to synchronize window closing with main proc */
void
winclosethread(void*)
{
    Window *w;

    threadsetname("winclosethread");

    for(;;){
        w = recvp(winclosechan);
        wclose(w);
    }
}
/*e: function winclosethread */

/*s: function deletethread */
/* thread to make Deleted windows that the client still holds disappear offscreen after an interval */
void
deletethread(void*)
{
    char *s;
    Image *i;

    threadsetname("deletethread");

    for(;;){
        s = recvp(deletechan);

        i = namedimage(display, s);
        if(i != nil){
            /* move it off-screen to hide it, since client is slow in letting it go */
            originwindow(i, i->r.min, view->r.max);
        }
        freeimage(i);
        free(s);
    }
}
/*e: function deletethread */

/*s: function deletetimeoutproc */
void
deletetimeoutproc(void *v)
{
    char *s = v;

    sleep(750);	/* remove window from screen after 3/4 of a second */
    sendp(deletechan, s);
}
/*e: function deletetimeoutproc */

/*s: function keyboardhide */
/*
 * Button 6 - keyboard toggle - has been pressed.
 * Send event to keyboard, wait for button up, send that.
 * Note: there is no coordinate translation done here; this
 * is just about getting button 6 to the keyboard simulator.
 */
void
keyboardhide(void)
{
    send(wkeyboard->mc.c, mouse);
    do
        readmouse(mousectl);
    while(mouse->buttons & (1<<5));
    send(wkeyboard->mc.c, mouse);
}
/*e: function keyboardhide */

/*s: enum Mxxx */
enum {
    MMouse,
    /*s: [[Mxxx]] cases */
    MReshape,
    /*e: [[Mxxx]] cases */
    NALT
};
/*e: enum Mxxx */

/*s: function mousethread */
void
mousethread(void*)
{
    /*s: [[mousethread()]] locals */
    // map<enum<Mxxx>, Alt>
    static Alt alts[NALT+1];
    /*x: [[mousethread()]] locals */
    Window *winput;
    Point xy; // logical coord
    /*x: [[mousethread()]] locals */
    bool sending = false;
    /*x: [[mousethread()]] locals */
    Mouse tmp;
    /*x: [[mousethread()]] locals */
    Window *w;
    /*x: [[mousethread()]] locals */
    bool moving = false;
    /*x: [[mousethread()]] locals */
    bool inside, band;
    Window *oin;
    Image *i;
    Rectangle r;
    /*x: [[mousethread()]] locals */
    bool scrolling = false;
    /*e: [[mousethread()]] locals */

    threadsetname("mousethread");

    /*s: [[mousethread()]] alts setup */
    // listen
    alts[MMouse].c = mousectl->c;
    alts[MMouse].v = &mousectl->Mouse;
    alts[MMouse].op = CHANRCV;
    /*x: [[mousethread()]] alts setup */
    alts[MReshape].c = mousectl->resizec;
    alts[MReshape].v = nil;
    alts[MReshape].op = CHANRCV;
    /*e: [[mousethread()]] alts setup */
    alts[NALT].op = CHANEND;

    for(;;)
        // message loop
        switch(alt(alts)){
        /*s: [[mousethread()]] event loop cases */
        case MMouse:
            /*s: [[mousethread()]] if wkeyboard and button 6 */
            if(wkeyboard!=nil && (mouse->buttons & (1<<5))){
                keyboardhide();
                break;
            }
            /*e: [[mousethread()]] if wkeyboard and button 6 */
        Again:
            winput = input;
            /*s: [[mousethread()]] if wkeyboard and ptinrect */
            /* override everything for the keyboard window */
            if(wkeyboard!=nil && ptinrect(mouse->xy, wkeyboard->screenr)){
                /* make sure it's on top; this call is free if it is */
                wtopme(wkeyboard);
                winput = wkeyboard;
            }
            /*e: [[mousethread()]] if wkeyboard and ptinrect */

            if(winput != nil && winput->i != nil){
                /* convert to logical coordinates */
                xy.x = mouse->xy.x + (winput->i->r.min.x - winput->screenr.min.x);
                xy.y = mouse->xy.y + (winput->i->r.min.y - winput->screenr.min.y);

                /*s: [[mousethread()]] goto Sending if scroll buttons */
                /* the up and down scroll buttons are not subject to the usual rules */
                if((mouse->buttons&(8|16)) && !winput->mouseopen)
                    goto Sending;
                /*e: [[mousethread()]] goto Sending if scroll buttons */

                inside = ptinrect(mouse->xy, insetrect(winput->screenr, Selborder));

                /*s: [[mousethread()]] set scrolling */
                if(winput->mouseopen)
                    scrolling = false;
                else 
                  if(scrolling)
                    scrolling = mouse->buttons;
                  else
                    scrolling = mouse->buttons && ptinrect(xy, winput->scrollr);
                /*e: [[mousethread()]] set scrolling */
                /*s: [[mousethread()]] set moving to true for some conditions */
                /* topped will be zero or less if window has been bottomed */
                if(sending == false && !scrolling 
                   && winborder(winput, mouse->xy) && winput->topped > 0){
                    moving = true;
                }
                /*e: [[mousethread()]] set moving to true for some conditions */
                else 
                   /*s: [[mousethread()]] set sending to true for some conditions */
                   if(inside && 
                      ((mouse->buttons&1) || winput->mouseopen || scrolling))
                        sending = true;
                   /*e: [[mousethread()]] set sending to true for some conditions */
            }else
                sending = false;

            /*s: [[mousethread()]] if sending */
            if(sending){
            Sending:
                /*s: [[mousethread()]] when sending mouse message to window, set the cursor */
                if(mouse->buttons == 0){
                    // cornercursor will call wsetcursor if cursor not on the border
                    cornercursor(winput, mouse->xy, false);
                    sending = false;
                }else
                    wsetcursor(winput, false);
                /*e: [[mousethread()]] when sending mouse message to window, set the cursor */

                tmp = mousectl->Mouse;
                tmp.xy = xy; // logical coordinates

                // Dispatch, to current window thread!
                send(winput->mc.c, &tmp);
                continue;
            }
            /*e: [[mousethread()]] if sending */
            /*s: [[mousethread()]] if not sending */
            w = wpointto(mouse->xy);

            /* change cursor if over anyone's border */
            if(w != nil)
                cornercursor(w, mouse->xy, false);
            else
                riosetcursor(nil, false);

            /*s: [[mousethread()]] if moving and buttons */
            if(moving && (mouse->buttons&7)){
                oin = winput;
                band = mouse->buttons & 3; // left or middle click

                sweeping = true;
                if(band)
                    i = bandsize(winput);
                else
                    i = drag(winput, &r);
                sweeping = false;

                if(i != nil){
                    if(winput == oin){
                        if(band)
                            wsendctlmesg(winput, Reshaped, i->r, i);
                        else
                            wsendctlmesg(winput, Moved, r, i);
                        cornercursor(winput, mouse->xy, true);
                    }else
                        freeimage(i);
                }
            }
            /*e: [[mousethread()]] if moving and buttons */

            if(w != nil)
                cornercursor(w, mouse->xy, false);

            /*s: [[mousethread()]] if buttons and was not sending */
            /* we're not sending the event, but if button is down maybe we should */
            if(mouse->buttons){
                /* w->topped will be zero or less if window has been bottomed */
                if(w==nil || (w==winput && w->topped > 0)){
                    if(mouse->buttons & 1){
                        ;
                    }else if(mouse->buttons & 2){
                        if(winput && !winput->mouseopen)
                            /*s: [[mousethread()]] middle click under certain conditions */
                            button2menu(winput);
                            /*e: [[mousethread()]] middle click under certain conditions */
                    }else if(mouse->buttons & 4)
                            /*s: [[mousethread()]] right click under certain conditions */
                            button3menu();
                            /*e: [[mousethread()]] right click under certain conditions */
                }else{
                    /* if button 1 event in the window, top the window and wait for button up. */
                    /* otherwise, top the window and pass the event on */
                    /*s: [[mousethread()]] click on unfocused window, set w */
                    w = wtop(mouse->xy);
                    /*e: [[mousethread()]] click on unfocused window, set w */
                    if(w && (mouse->buttons!=1 || winborder(w, mouse->xy)))
                        // input changed
                        goto Again;

                    goto Drain;
                }
            }
            /*e: [[mousethread()]] if buttons and was not sending */
            moving = false;
            break;
            /*e: [[mousethread()]] if not sending */

        /*s: [[mousethread()]] Drain label */
        Drain:
            do {
                readmouse(mousectl);
            } while(mousectl->buttons);
            moving = false;
            goto Again;	/* recalculate mouse position, cursor */
        /*e: [[mousethread()]] Drain label */
        /*x: [[mousethread()]] event loop cases */
        case MReshape:
            resized();
            break;
        /*e: [[mousethread()]] event loop cases */
        }
}
/*e: function mousethread */

/*s: function resized */
void
resized(void)
{
    Image *im;
    int i, j, ishidden;
    Rectangle r;
    Point o, n;
    Window *w;

    if(getwindow(display, Refnone) < 0)
        error("failed to re-attach window");

    freescrtemps();
    freescreen(wscreen);

    wscreen = allocscreen(view, background, 0);
    if(wscreen == nil)
        error("can't re-allocate screen");

    draw(view, view->r, background, nil, ZP);
    o = subpt(viewr.max, viewr.min);
    n = subpt(view->clipr.max, view->clipr.min);

    for(i=0; i<nwindow; i++){
        w = windows[i];
        /*s: [[resized()]] continue if window was deleted */
        if(w->deleted)
            continue;
        /*e: [[resized()]] continue if window was deleted */
        r = rectsubpt(w->i->r, viewr.min);
        r.min.x = (r.min.x*n.x)/o.x;
        r.min.y = (r.min.y*n.y)/o.y;
        r.max.x = (r.max.x*n.x)/o.x;
        r.max.y = (r.max.y*n.y)/o.y;
        r = rectaddpt(r, view->clipr.min);
        ishidden = 0;
        for(j=0; j<nhidden; j++)
            if(w == hidden[j]){
                ishidden = 1;
                break;
            }
        if(ishidden){
            im = allocimage(display, r, view->chan, 0, DWhite);
            r = ZR;
        }else
            im = allocwindow(wscreen, r, Refbackup, DWhite);
        if(im)
            wsendctlmesg(w, Reshaped, r, im);
    }
    viewr = view->r;
    flushimage(display, 1);
}
/*e: function resized */

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

/*s: function button2menu */
void
button2menu(Window *w)
{
    /*s: [[button2menu()]] return if window was deleted */
    if(w->deleted)
        return;
    /*e: [[button2menu()]] return if window was deleted */

    incref(w);
    /*s: [[button2menu()]] menu2str adjustments for scrolling */
    if(w->scrolling)
        menu2str[Scroll] = "noscroll";
    else
        menu2str[Scroll] = "scroll";
    /*e: [[button2menu()]] menu2str adjustments for scrolling */
    switch(menuhit(2, mousectl, &menu2, wscreen)){
    /*s: [[button2menu()]] cases */
    case Scroll:
        if(w->scrolling ^= 1)
            wshow(w, w->nr);
        break;
    /*x: [[button2menu()]] cases */
    case Cut:
        wsnarf(w);
        wcut(w);
        wscrdraw(w);
        break;

    case Snarf:
        wsnarf(w);
        break;

    case Paste:
        getsnarf();
        wpaste(w);
        wscrdraw(w);
        break;

    case Send:
        getsnarf();
        wsnarf(w);
        if(nsnarf == 0)
            break;
        if(w->rawing){
            waddraw(w, snarf, nsnarf);
            if(snarf[nsnarf-1]!='\n' && snarf[nsnarf-1]!='\004')
                          waddraw(w, L"\n", 1);
        }else{
            winsert(w, snarf, nsnarf, w->nr);
            if(snarf[nsnarf-1]!='\n' && snarf[nsnarf-1]!='\004')
                winsert(w, L"\n", 1, w->nr);
        }
        wsetselect(w, w->nr, w->nr);
        wshow(w, w->nr);
        break;
    /*x: [[button2menu()]] cases */
    case Plumb:
        wplumb(w);
        break;
    /*e: [[button2menu()]] cases */
    }
    wclose(w); // decref

    wsendctlmesg(w, Wakeup, ZR, nil);
    flushimage(display, true);
}
/*e: function button2menu */

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
/*e: windows/rio/rio.c */
