/*s: lib_gui/libpanel/init.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"

/*s: function [[plinit]] */
/*
 * Just a wrapper for all the initialization routines
 */
error0 plinit(int ldepth){
    if(pl_drawinit(ldepth) == ERROR_0) return ERROR_0;
    return OK_1;
}
/*e: function [[plinit]] */
/*e: lib_gui/libpanel/init.c */
