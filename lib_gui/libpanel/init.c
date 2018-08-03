/*s: lib_gui/libpanel/init.c */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

/*s: function [[plinit]] */
/*
 * Just a wrapper for all the initialization routines
 */
error0 plinit(int ldepth){
    return pl_drawinit(ldepth);
}
/*e: function [[plinit]] */
/*e: lib_gui/libpanel/init.c */
