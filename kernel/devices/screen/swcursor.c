/*s: kernel/devices/screen/swcursor.c */
/*
 * Software cursor
 */
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

#include    "../port/portscreen.h"



/*s: global swcursor_arrow */
Cursor  swcursor_arrow = {
    .offset = { -1, -1 },
    .clr = { 
      0xFF, 0xFF, 0x80, 0x01, 0x80, 0x02, 0x80, 0x0C, 
      0x80, 0x10, 0x80, 0x10, 0x80, 0x08, 0x80, 0x04, 
      0x80, 0x02, 0x80, 0x01, 0x80, 0x02, 0x8C, 0x04, 
      0x92, 0x08, 0x91, 0x10, 0xA0, 0xA0, 0xC0, 0x40, 
    },
    .set = { 
      0x00, 0x00, 0x7F, 0xFE, 0x7F, 0xFC, 0x7F, 0xF0, 
      0x7F, 0xE0, 0x7F, 0xE0, 0x7F, 0xF0, 0x7F, 0xF8, 
      0x7F, 0xFC, 0x7F, 0xFE, 0x7F, 0xFC, 0x73, 0xF8, 
      0x61, 0xF0, 0x60, 0xE0, 0x40, 0x40, 0x00, 0x00, 
    },
};
/*e: global swcursor_arrow */

/*s: global swback */
Memimage*   swback; /* screen under cursor */
/*e: global swback */
/*s: global swimg1 */
Memimage*   swimg1;
/*e: global swimg1 */
/*s: global swmask1 */
Memimage*   swmask1;
/*e: global swmask1 */

/*s: global swpt */
Point   swpt;   /* desired cursor location */
/*e: global swpt */
/*s: global swvispt */
Point   swvispt;    /* actual cursor location */
/*e: global swvispt */
/*s: global swvers */
int swvers; /* incremented each time cursor image changes */
/*e: global swvers */
/*s: global swvisvers */
int swvisvers;  /* the version on the screen */
/*e: global swvisvers */

/*s: global swenabled */
bool swenabled;  /* is the cursor supposed to be on the screen? */
/*e: global swenabled */
/*s: global swvisible */
bool swvisible;  /* is the cursor visible? */
/*e: global swvisible */

/*s: global swimg */
Memimage*   swimg;  /* cursor image */
/*e: global swimg */
/*s: global swmask */
Memimage*   swmask; /* cursor mask */
/*e: global swmask */

/*s: global swrect */
Rectangle   swrect; /* screen rectangle in swback */
/*e: global swrect */

/*s: global swoffset */
Point   swoffset;
/*e: global swoffset */

void swcursor_clock(void);


/*s: function swcursorinit */
void
swcursor_init(void)
{
    static bool init;
    /*s: [[swcursorinit()]] other locals */
    static bool warned;
    /*e: [[swcursorinit()]] other locals */

    if(!init){
        init = true;
        addclock0link(swcursor_clock, 10);
        //swenabled = 1; //bcm: was not in pc (but maybe bug?)
    }
    /*s: [[swcursorinit()]] free old versions of cursor images if any */
    if(swback){
        freememimage(swback);
        freememimage(swmask);
        freememimage(swmask1);
        freememimage(swimg);
        freememimage(swimg1);
    }
    /*e: [[swcursorinit()]] free old versions of cursor images if any */

    swback  = allocmemimage(Rect(0,0,32,32), gscreen->chan);

    swmask  = allocmemimage(Rect(0,0,16,16), GREY8);
    swmask1 = allocmemimage(Rect(0,0,16,16), GREY1);
    swimg   = allocmemimage(Rect(0,0,16,16), GREY8);
    swimg1  = allocmemimage(Rect(0,0,16,16), GREY1);

    /*s: [[swcursorinit()]] sanity check cursor images */
    if(swback == nil || swmask == nil || swmask1 == nil || swimg == nil || swimg1 == nil){
        print("software cursor: allocmemimage fails");
        return;
    }
    /*e: [[swcursorinit()]] sanity check cursor images */

    memfillcolor(swmask,  DOpaque);
    memfillcolor(swmask1, DOpaque);
    memfillcolor(swimg,   DBlack);
    memfillcolor(swimg1,  DBlack);
}
/*e: function swcursorinit */

/*s: function swcursorclock */
void
swcursor_clock(void)
{
    int x;

    if(!swenabled)
        return;
    //swmove(mousexy()); //bcm: only, was not in pc
    if(swvisible && eqpt(swpt, swvispt) && swvers==swvisvers)
        return;

    x = arch_splhi();
    // check again, might have changed in between
    if(swenabled)
     if(!swvisible || !eqpt(swpt, swvispt) || swvers!=swvisvers)
      if(canqlock(&drawlock)){

        swcursor_hide();
        swcursor_draw();

        qunlock(&drawlock);
    }
    arch_splx(x);
}
/*e: function swcursorclock */

/*s: function swcursordraw */
void
swcursor_draw(void)
{
 bool dounlock;

    if(swvisible)
        return;
    if(!swenabled)
        return;
    if(swback == nil || swimg1 == nil || swmask1 == nil)
        return;
    //dounlock = canqlock(&drawlock); //bcm:
    assert(!canqlock(&drawlock)); // pc:

    swvispt = swpt;
    swvisvers = swvers;
    // cursor is 16x16 picture
    swrect = rectaddpt(Rect(0,0,16,16), swvispt);
    // save what is under the cursor
    memimagedraw(swback, swback->r, gscreen, swpt, memopaque, ZP, S);
    // draw cursor
    memimagedraw(gscreen, swrect, swimg1, ZP, swmask1, ZP, SoverD);
    arch_flushmemscreen(swrect);
    swvisible = true;
    //bcm:
    //if(dounlock)
    //    qunlock(&drawlock);

}
/*e: function swcursordraw */

/*s: function swcursorhide */
/*
 * called with drawlock locked for us, most of the time.
 * kernel prints at inopportune times might mean we don't
 * hold the lock, but memimagedraw is now reentrant so
 * that should be okay: worst case we get cursor droppings.
 */
void
swcursor_hide(void)
{
    if(!swvisible)
        return;
    if(swback == nil)
        return;

    swvisible = false;
    // restore what was under the cursor
    memimagedraw(gscreen, swrect, swback, ZP, memopaque, ZP, S);
    arch_flushmemscreen(swrect);
}
/*e: function swcursorhide */

/*s: function swcursoravoid */
void
swcursor_avoid(Rectangle r)
{
    if(swvisible && rectXrect(r, swrect))
        swcursor_hide();
}
/*e: function swcursoravoid */

/*s: function swload */
void
swload(Cursor *curs)
{
    byte *ip, *mp;
    int i, j, set, clr;

    if(!swimg || !swmask || !swimg1 || !swmask1)
        return;
    /*
     * Build cursor image and mask.
     * Image is just the usual cursor image
     * but mask is a transparent alpha mask.
     * 
     * The 16x16x8 memimages do not have
     * padding at the end of their scan lines.
     */
    ip = byteaddr(swimg, ZP);
    mp = byteaddr(swmask, ZP);

    for(i=0; i<32; i++){
        set = curs->set[i];
        clr = curs->clr[i];
        for(j=0x80; j; j>>=1){
            *ip++ = set & j ? 0x00 : 0xFF;
            *mp++ = (clr|set) & j ? 0xFF : 0x00;
        }
    }
    swoffset = curs->offset;
    swvers++;

    memimagedraw(swimg1,  swimg1->r,  swimg,  ZP, memopaque, ZP, S);
    memimagedraw(swmask1, swmask1->r, swmask, ZP, memopaque, ZP, S);
}
/*e: function swload */

/*s: function swmove */
int
swmove(Point p)
{
    swpt = addpt(p, swoffset);
    return 0;
}
/*e: function swmove */
/*e: kernel/devices/screen/swcursor.c */
