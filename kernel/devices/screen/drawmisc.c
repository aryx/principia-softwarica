/*s: kernel/devices/screen/drawmisc.c */
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

/*s: global blanktime */
ulong blanktime = 30;   /* in minutes; a half hour */
/*e: global blanktime */

/*s: function drawrefactive */
int
drawrefactive(void *a)
{
    Client *c;

    c = a;
    return c->refreshme || c->refresh != nil;
}
/*e: function drawrefactive */

/*s: function drawrefreshscreen */
void
drawrefreshscreen(DImage *l, Client *client)
{
    while(l != nil && l->dscreen == nil)
        l = l->fromname;
    if(l != nil && l->dscreen->owner != client)
        l->dscreen->owner->refreshme = 1;
}
/*e: function drawrefreshscreen */

/*s: function drawrefresh */
void
drawrefresh(Memimage*, Rectangle r, void *v)
{
    Refx *x;
    DImage *d;
    Client *c;
    Refresh *ref;

    if(v == 0)
        return;
    x = v;
    c = x->client;
    d = x->dimage;
    for(ref=c->refresh; ref; ref=ref->next)
        if(ref->dimage == d){
            combinerect(&ref->r, r);
            return;
        }
    ref = malloc(sizeof(Refresh));
    if(ref){
        ref->dimage = d;
        ref->r = r;
        ref->next = c->refresh;
        c->refresh = ref;
    }
}
/*e: function drawrefresh */



/*s: function drawcmap */
/*
 * On 8 bit displays, load the default color map
 */
void
drawcmap(void)
{
    int r, g, b;
    int cr, cg, cb, v;
    int num, den;
    int i, j;

    drawactive(true);  /* to restore map from backup */
    for(r=0,i=0; r!=4; r++)
        for(v=0; v!=4; v++,i+=16){
        for(g=0,j=v-r; g!=4; g++)
            for(b=0;b!=4;b++,j++){
            den = r;
            if(g > den)
                den = g;
            if(b > den)
                den = b;
            if(den == 0)    /* divide check -- pick grey shades */
                cr = cg = cb = v*17;
            else{
                num = 17*(4*den+v);
                cr = r*num/den;
                cg = g*num/den;
                cb = b*num/den;
            }
            setcolor(i+(j&15),
                cr*0x01010101, cg*0x01010101, cb*0x01010101);
            }
    }
}
/*e: function drawcmap */

/*s: function drawblankscreen */
void
drawblankscreen(bool blank)
{
    int i, nc;
    ulong *p;

    if(blank == sdraw.blanked)
        return;
    if(!candlock())
        return;
    if(screenimage == nil){
        dunlock();
        return;
    }
    p = sdraw.savemap;
    nc = screenimage->depth > 8 ? 256 : 1<<screenimage->depth;

    /*
     * blankscreen uses the hardware to blank the screen
     * when possible.  to help in cases when it is not possible,
     * we set the color map to be all black.
     */
    if(!blank){ /* turn screen on */
        for(i=0; i<nc; i++, p+=3)
            setcolor(i, p[0], p[1], p[2]);
        blankscreen(false);
    }else{  /* turn screen off */
        blankscreen(true);
        for(i=0; i<nc; i++, p+=3){
            getcolor(i, &p[0], &p[1], &p[2]);
            setcolor(i, 0, 0, 0);
        }
    }
    sdraw.blanked = blank;
    dunlock();
}
/*e: function drawblankscreen */

/*s: function drawactive */
/*
 * record activity on screen, changing blanking as appropriate
 */
void
drawactive(bool active)
{
    if(active){
        drawblankscreen(false);
        sdraw.blanktime = CPUS(0)->ticks;
    }else{
        if(blanktime && sdraw.blanktime 
           && TK2SEC(CPUS(0)->ticks - sdraw.blanktime)/60 >= blanktime)
            drawblankscreen(true);
    }
}
/*e: function drawactive */

/*s: function drawidletime */
int
drawidletime(void)
{
    return TK2SEC(CPUS(0)->ticks - sdraw.blanktime)/60;
}
/*e: function drawidletime */

/*e: kernel/devices/screen/drawmisc.c */
