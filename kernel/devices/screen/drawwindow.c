/*s: kernel/devices/screen/drawwindow.c */
#include    "u.h"
#include    "../port/lib.h"
#include    "../port/error.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"

#include    <draw.h>
#include    <memdraw.h>
#include    <memlayer.h>

#include    "devdraw.h"

/*s: global dscreen */
// list<ref<DScreen>> (next = DScreen.next)
static  DScreen*    dscreen;
/*e: global dscreen */

/*s: function drawlookupdscreen */
DScreen*
drawlookupdscreen(int id)
{
    DScreen *s;

    s = dscreen;
    while(s){
        if(s->id == id)
            return s;
        s = s->next;
    }
    return nil;
}
/*e: function drawlookupdscreen */

/*s: function drawlookupscreen */
DScreen*
drawlookupscreen(Client *client, int id, CScreen **cs)
{
    CScreen *s;

    s = client->cscreen;
    while(s){
        if(s->dscreen->id == id){
            *cs = s;
            return s->dscreen;
        }
        s = s->next;
    }
    error(Enodrawscreen);
    return nil;
}
/*e: function drawlookupscreen */

/*s: function drawinstallscreen */
Memscreen*
drawinstallscreen(Client *client, DScreen *d, int id, DImage *dimage, DImage *dfill, bool public)
{
    Memscreen *s;
    CScreen *c;

    c = malloc(sizeof(CScreen));
    /*s: [[drawinstallscreen()]] sanity check dimage */
    if(dimage && dimage->image && dimage->image->chan == 0)
        panic("bad image %p in drawinstallscreen", dimage->image);
    /*e: [[drawinstallscreen()]] sanity check dimage */
    /*s: [[drawinstallscreen()]] sanity check c */
    if(c == nil)
        return nil;
    /*e: [[drawinstallscreen()]] sanity check c */

    if(d == nil){
        d = malloc(sizeof(DScreen));
        /*s: [[drawinstallscreen()]] sanity check d */
        if(d == nil){
            free(c);
            return nil;
        }
        /*e: [[drawinstallscreen()]] sanity check d */
        s = malloc(sizeof(Memscreen));
        /*s: [[drawinstallscreen()]] sanity check s */
        if(s == nil){
            free(c);
            free(d);
            return nil;
        }
        /*e: [[drawinstallscreen()]] sanity check s */

        d->id = id;
        d->dimage = dimage;
        d->dfill = dfill;

        d->screen = s;
        if(dimage){
            s->image = dimage->image;
            dimage->ref++;
        }
        if(dfill){
            s->fill = dfill->image;
            dfill->ref++;
        }
        // no windows yet
        s->frontmost = nil;
        s->rearmost = nil;

        d->public = public;
        d->owner = client;
        d->ref = 0;

        // add_list(d, dscreen)
        d->next = dscreen;
        dscreen = d;
    }

    c->dscreen = d;
    d->ref++;

    // add_list(c, client->cscreen)
    c->next = client->cscreen;
    client->cscreen = c;

    return d->screen;
}
/*e: function drawinstallscreen */

/*s: function drawfreedscreen */
void
drawfreedscreen(DScreen *this)
{
    DScreen *ds, *next;

    this->ref--;
    /*s: [[drawfreedscreen()]] sanity check reference count */
    if(this->ref < 0)
        print("negative ref in drawfreedscreen\n");
    /*e: [[drawfreedscreen()]] sanity check reference count */
    if(this->ref > 0)
        return;
    // else

    // remove_list(this, dscreen)
    ds = dscreen;
    if(ds == this){
        dscreen = this->next;
        goto Found;
    }
    while(next = ds->next){ /* assign = */
        if(next == this){
            ds->next = this->next;
            goto Found;
        }
        ds = next;
    }
    error(Enodrawimage);

    Found:
    if(this->dimage)
        drawfreedimage(this->dimage);
    if(this->dfill)
        drawfreedimage(this->dfill);
    free(this->screen);
    free(this);
}
/*e: function drawfreedscreen */

/*s: function drawuninstallscreen */
void
drawuninstallscreen(Client *client, CScreen *this)
{
    CScreen *cs, *next;

    // remove_list(this, client->cscreen)
    cs = client->cscreen;
    if(cs == this){
        client->cscreen = this->next;

        drawfreedscreen(this->dscreen);
        free(this);
        return;
    }
    while(next = cs->next){ /* assign = */
        if(next == this){
            cs->next = this->next;
     
            drawfreedscreen(this->dscreen);
            free(this);
            return;
        }
        cs = next;
    }
}
/*e: function drawuninstallscreen */


/*e: kernel/devices/screen/drawwindow.c */
