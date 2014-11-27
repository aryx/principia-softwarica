/*s: kernel/devices/screen/screen.c */
#include    "u.h"
#include    "../port/lib.h"
#include    "../port/error.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"

#include    <draw.h>
#include    <memdraw.h>
#include    <memlayer.h>
#include    <cursor.h>

#include    "screen.h"

// many of the stuff below used to be in vgascreen.c, but they are
// quite VGA independent so better to have a generic screen.c

/*s: global gscreen */
Memimage *gscreen;
/*e: global gscreen */
/*s: global gscreendata */
Memdata gscreendata;
/*e: global gscreendata */
/*s: global physgscreenr */
Rectangle physgscreenr;
/*e: global physgscreenr */

//put cursor here?
// software cursor?
// curpos, window, graphic text mode? graphic_screenputs
/*e: kernel/devices/screen/screen.c */
