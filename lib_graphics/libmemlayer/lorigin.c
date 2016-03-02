/*s: lib_graphics/libmemlayer/lorigin.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <memlayer.h>

/*s: function memlorigin */
/*
 * Place i so i->r.min = log, i->layer->screenr.min == scr.
*/
errorneg1
memlorigin(Memimage *i, Point log, Point scr)
{
    Memlayer *l;
    Memscreen *s;
    Rectangle newr, oldr;
    Point delta;
    bool eqlog, eqscr, wasclear;
    /*s: [[memlorigin()]] other locals */
    Memimage *nsave;
    /*x: [[memlorigin()]] other locals */
    Memimage *shad, *t;
    bool overlap;
    Rectangle x;
    /*e: [[memlorigin()]] other locals */

    l = i->layer;
    s = l->screen;

    oldr = l->screenr;
    newr = Rect(scr.x, scr.y, scr.x + Dx(oldr), scr.y + Dy(oldr));

    eqscr = eqpt(scr, oldr.min);
    eqlog = eqpt(log, i->r.min);

    if(eqscr && eqlog)
        return 0;

    /*s: [[memlorigin()]] allocate new save image if log changed */
    nsave = nil;
    if(!eqlog && l->save != nil){
        nsave = allocmemimage(Rect(log.x, log.y, log.x+Dx(oldr), log.y+Dy(oldr)), i->chan);
        /*s: [[memlorigin()]] sanity check nsave */
        if(nsave == nil)
            return ERROR_NEG1;
        /*e: [[memlorigin()]] sanity check nsave */
    }
    /*e: [[memlorigin()]] allocate new save image if log changed */
    /*
     * Bring it to front and move logical coordinate system.
     */
    memltofront(i);
    wasclear = l->clear;
    /*s: [[memlorigin()]] set new save image if log changed */
    if(nsave){
        if(!wasclear)
            memimagedraw(nsave, nsave->r, l->save, l->save->r.min, nil, ZP, S);
        freememimage(l->save);
        l->save = nsave;
    }
    /*e: [[memlorigin()]] set new save image if log changed */

    // like in user side
    delta = subpt(log, i->r.min);
    // new image coords
    i->r     = rectaddpt(i->r, delta);
    i->clipr = rectaddpt(i->clipr, delta);

    l->delta = subpt(l->screenr.min, i->r.min);

    if(eqscr)
        return 0;
    // else

    /*s: [[memlorigin()]] move window */
    /*
     * To clean up old position, make a shadow window there, don't paint it,
     * push it behind this one, and (later) delete it.  Because the refresh
     * function for this fake window is a no-op, this will cause no graphics
     * action except to restore the background and expose the windows
     * previously hidden.
     */
    shad = memlalloc(s, oldr, memlnorefresh, nil, DNofill);
    /*s: [[memlorigin()]] sanity check shad */
    if(shad == nil)
        return ERROR_NEG1;
    /*e: [[memlorigin()]] sanity check shad */
    /*s: [[memlorigin()]] manage stack of windows, put shad after front */
    // add_after_double_list(shad, i, s->frontmost, s->rearmost)
    s->frontmost = i; // useless since memltofront(i) above
    if(s->rearmost == i)
        s->rearmost = shad;
    else
        l->rear->layer->front = shad;
    shad->layer->front = i;
    shad->layer->rear = l->rear;
    l->rear = shad;
    l->front = nil;
    /*e: [[memlorigin()]] manage stack of windows, put shad after front */
    shad->layer->clear = false;

    /*
     * Shadow is now holding down the fort at the old position.
     * Move the window and hide things obscured by new position.
     */
    for(t = l->rear->layer->rear; t != nil; t = t->layer->rear){
        x = newr;
        overlap = rectclip(&x, t->layer->screenr);
        if(overlap){
            memlhide(t, x);
            t->layer->clear = false;
        }
    }
    l->screenr = newr;
    l->delta = subpt(scr, i->r.min);
    l->clear = rectinrect(newr, l->screen->image->clipr);

    /*
     * Everything's covered.  Copy to new position and delete shadow window.
     */
    if(wasclear)
        memdraw(s->image, newr, s->image, oldr.min, nil, ZP, S);
    else
        memlexpose(i, newr);

    memldelete(shad);

    return 1;
    /*e: [[memlorigin()]] move window */
}
/*e: function memlorigin */

/*s: function memlnorefresh */
void
memlnorefresh(Memimage *l, Rectangle r, void *v)
{
    USED(l);
    USED(r.min.x);
    USED(v);
}
/*e: function memlnorefresh */
/*e: lib_graphics/libmemlayer/lorigin.c */
