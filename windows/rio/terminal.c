/*s: windows/rio/terminal.c */
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

#include "dat.h"
#include "fns.h"



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

/*e: windows/rio/terminal.c */
