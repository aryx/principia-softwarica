/*s: windows/libpanel/init.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"
/*s: function plinit */
/*
 * Just a wrapper for all the initialization routines
 */
int plinit(int ldepth){
    if(!pl_drawinit(ldepth)) return 0;
    return 1;
}
/*e: function plinit */
/*e: windows/libpanel/init.c */
