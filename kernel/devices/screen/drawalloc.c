/*s: kernel/devices/screen/drawalloc.c */
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

#include    "devdraw.h"

/*s: function allocdimage */
/// makescreenimage | drawinstall -> <>
DImage*
allocdimage(Memimage *i)
{
    DImage *d;

    d = malloc(sizeof(DImage));
    /*s: [[allocdimage()]] sanity check d */
    if(d == nil)
        return nil;
    /*e: [[allocdimage()]] sanity check d */
    d->image = i;
    d->ref = 1;

    d->name = nil;
    d->fromname = nil;
    d->vers = 0;
    d->nfchar = 0;
    d->fchar = 0;

    return d;
}
/*e: function allocdimage */

/*s: function drawinstall */
Memimage*
drawinstall(Client *client, int id, Memimage *i, DScreen *dscreen)
{
    DImage *d;

    d = allocdimage(i);
    /*s: [[drawinstall()]] sanity check d */
    if(d == nil)
        return nil;
    /*e: [[drawinstall()]] sanity check d */
    d->id = id;
    /*s: [[drawinstall()]] install dscreen */
    d->dscreen = dscreen;
    /*e: [[drawinstall()]] install dscreen */

    // insert_hash(d, client->dimage)
    d->next = client->dimage[id&HASHMASK];
    client->dimage[id&HASHMASK] = d;

    return i;
}
/*e: function drawinstall */

/*s: function drawfreedimage */
void
drawfreedimage(DImage *dimage)
{
    int i;
    Memimage *l;
    DScreen *ds;

    dimage->ref--;
    /*s: [[drawfreedimage()]] sanity check dimage ref */
    if(dimage->ref < 0)
        print("negative ref in drawfreedimage\n");
    /*e: [[drawfreedimage()]] sanity check dimage ref */
    if(dimage->ref > 0)
        return;
    // else

    /*s: [[drawfreedimage()]] free names */
    /* any names? */
    for(i=0; i<sdraw.nname; )
        if(sdraw.name[i].dimage == dimage)
            drawdelname(sdraw.name+i);
        else
            i++;

    if(dimage->fromname){   /* acquired by name; owned by someone else*/
        drawfreedimage(dimage->fromname);
        goto Return;
    }
    /*e: [[drawfreedimage()]] free names */
    /*s: [[drawfreedimage()]] if dscreen */
    ds = dimage->dscreen;
    if(ds){
        l = dimage->image;
        /*s: [[drawfreedimage()]] addflush */
        if(l->data == screenimage->data)
            addflush(l->layer->screenr);
        /*e: [[drawfreedimage()]] addflush */
        /*s: [[drawfreedimage()]] free refreshptr */
        if(l->layer->refreshfn == drawrefresh)  /* else true owner will clean up */
            free(l->layer->refreshptr);
        l->layer->refreshptr = nil;
        /*e: [[drawfreedimage()]] free refreshptr */

        if(drawgoodname(dimage))
            memldelete(l);
        else
            memlfree(l);

        drawfreedscreen(ds); // one less reference
    }
    /*e: [[drawfreedimage()]] if dscreen */
    else
        freememimage(dimage->image);
    Return:
    free(dimage->fchar);
    free(dimage);
}
/*e: function drawfreedimage */

/*s: function drawuninstall */
void
drawuninstall(Client *client, int id)
{
    DImage *d, *next;

    d = client->dimage[id&HASHMASK];
    /*s: [[drawuninstall()]] sanity check d */
    if(d == nil)
        error(Enodrawimage);
    /*e: [[drawuninstall()]] sanity check d */

    // hash_remove(client->dimage, id)
    if(d->id == id){
        client->dimage[id&HASHMASK] = d->next;

        drawfreedimage(d);
        return;
    }
    while(next = d->next){  /* assign = */
        if(next->id == id){
            d->next = next->next;

            drawfreedimage(next);
            return;
        }
        d = next;
    }

    error(Enodrawimage);
}
/*e: function drawuninstall */

/*e: kernel/devices/screen/drawalloc.c */
