/*s: windows/rio/cursor.c */
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

/*s: global lastcursor */
Cursor	*lastcursor;
/*e: global lastcursor */

/*s: function riosetcursor */
void
riosetcursor(Cursor *p, bool force)
{
    if(!force && p==lastcursor)
        return;
    setcursor(mousectl, p);
    lastcursor = p;
}
/*e: function riosetcursor */
/*e: windows/rio/cursor.c */
