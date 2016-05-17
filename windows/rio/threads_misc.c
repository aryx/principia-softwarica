/*s: windows/rio/threads_misc.c */
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

/*e: windows/rio/threads_misc.c */
