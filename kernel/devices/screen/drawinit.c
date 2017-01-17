/*s: kernel/devices/screen/drawinit.c */
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
#include    "portscreen.h"

#include    "devdraw.h"

/*s: global screenimage */
Memimage    *screenimage;
/*e: global screenimage */
/*s: global screendimage */
static  DImage* screendimage;
/*e: global screendimage */

/*s: global screenname */
char    screenname[40];
/*e: global screenname */
/*s: global screennameid */
// gensym
static  int screennameid;
/*e: global screennameid */


/*s: function makescreenimage */
static DImage*
makescreenimage(void)
{
    /*s: [[makescreenimage()]] locals */
    Memdata *md;
    Memimage *i;
    DImage *di;
    /*x: [[makescreenimage()]] locals */
    int width, depth;
    Rectangle r;
    ulong chan;
    /*e: [[makescreenimage()]] locals */

    /*s: [[makescreenimage()]] allocate Memdata md */
    // allocate Memdata
    md = malloc(sizeof(Memdata));
    /*s: [[makescreenimage()]] sanity check md */
    if(md == nil)
        return nil;
    /*e: [[makescreenimage()]] sanity check md */
    md->allocd = true;

    md->bdata = attachscreen(&r, &chan, &depth, &width, &sdraw.softscreen);
    /*s: [[makescreenimage()]] sanity check md bdata */
    if(md->bdata == nil){
        free(md);
        return nil;
    }
    /*e: [[makescreenimage()]] sanity check md bdata */
    md->base = nil; // not allocated by poolalloc
    md->ref = 1;
    /*e: [[makescreenimage()]] allocate Memdata md */
    /*s: [[makescreenimage()]] allocate Memimage i */
    // allocate Memimage
    i = allocmemimaged(r, chan, md);
    /*s: [[makescreenimage()]] sanity check i */
    if(i == nil){
        free(md);
        return nil;
    }
    /*e: [[makescreenimage()]] sanity check i */
    /*e: [[makescreenimage()]] allocate Memimage i */
    /*s: [[makescreenimage()]] allocate DImage di */
    // allocate DImage
    di = allocdimage(i);
    /*s: [[makescreenimage()]] sanity check di */
    if(di == nil){
        freememimage(i);    /* frees md */
        return nil;
    }
    /*e: [[makescreenimage()]] sanity check di */
    /*e: [[makescreenimage()]] allocate DImage di */

    /*s: [[makescreenimage()]] drawaddname */
    if(!waserror()){
        snprint(screenname, sizeof screenname, "noborder.screen.%d", ++screennameid);
        drawaddname(nil, di, strlen(screenname), screenname);
        poperror();
    }
    /*e: [[makescreenimage()]] drawaddname */

    return di;
}
/*e: function makescreenimage */

/*s: function initscreenimage */
error0
initscreenimage(void)
{

    /*s: [[initscreenimage()]] only once guard */
    if(screenimage != nil)
        return OK_1;
    /*e: [[initscreenimage()]] only once guard */

    screendimage = makescreenimage();
    /*s: [[initscreenimage()]] sanity check screendimage */
    if(screendimage == nil)
        return ERROR_0;
    /*e: [[initscreenimage()]] sanity check screendimage */
    screenimage = screendimage->image;

    /*s: [[initscreenimage()]] other initializations */
    mouseresize();
    /*e: [[initscreenimage()]] other initializations */
    return OK_1;
}
/*e: function initscreenimage */

/*s: function deletescreenimage */
void
deletescreenimage(void)
{
    dlock();
    if(screenimage){
        /* will be freed via screendimage; disable */
        screenimage->clipr = ZR;
        screenimage = nil;
    }
    if(screendimage){
        drawfreedimage(screendimage);
        screendimage = nil;
    }
    dunlock();
}
/*e: function deletescreenimage */

/*s: function resetscreenimage */
void
resetscreenimage(void)
{
    dlock();
    initscreenimage();
    dunlock();
}
/*e: function resetscreenimage */

/*e: kernel/devices/screen/drawinit.c */
