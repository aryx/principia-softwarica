/*s: windows/rio/rio.c */
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

/*s: global fontname */
char		*fontname;
/*e: global fontname */

/*s: global kbdargv */
char *kbdargv[] = { "rc", "-c", nil, nil };
/*e: global kbdargv */


/*s: function usage */
void
usage(void)
{
    fprint(STDERR, "usage: rio [-f font] [-i initcmd] [-k kbdcmd] [-s]\n");
    exits("usage");
}
/*e: function usage */


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

/*s: function killprocs */
void
killprocs(void)
{
    int i;

    for(i=0; i<nwindow; i++)
        postnote(PNGROUP, windows[i]->pid, "hangup");
}
/*e: function killprocs */


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

    // Rio, a graphical application

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

    desktop = allocscreen(view, background, false);
    /*s: [[main()]] sanity check desktop */
    if(desktop == nil)
        error("can't allocate desktop");
    /*e: [[main()]] sanity check desktop */

    draw(view, viewr, background, nil, ZP);
    flushimage(display, true);
    /*e: [[main()]] graphics initializations */

    // Rio, a concurrent application

    /*s: [[main()]] communication channels creation */
    exitchan     = chancreate(sizeof(int), 0);
    /*x: [[main()]] communication channels creation */
    deletechan   = chancreate(sizeof(char*), 0);
    /*x: [[main()]] communication channels creation */
    winclosechan = chancreate(sizeof(Window*), 0);
    /*e: [[main()]] communication channels creation */
    /*s: [[main()]] threads creation */
    threadcreate(keyboardthread, nil, STACK);
    threadcreate(mousethread, nil, STACK);
    /*x: [[main()]] threads creation */
    threadcreate(deletethread, nil, STACK);
    /*x: [[main()]] threads creation */
    threadcreate(winclosethread, nil, STACK);
    /*x: [[main()]] threads creation */
    timerinit();
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
            i = allocwindow(desktop, r, Refbackup, DWhite);
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
/*e: windows/rio/rio.c */
