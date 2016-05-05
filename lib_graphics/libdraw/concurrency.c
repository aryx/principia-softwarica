/*s: lib_graphics/libdraw/concurrency.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: global debuglockdisplay */
bool		debuglockdisplay = false;
/*e: global debuglockdisplay */

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

/*e: lib_graphics/libdraw/concurrency.c */
