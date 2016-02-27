/*s: lib_graphics/libmemlayer/lalloc.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <memlayer.h>

/*s: function memlalloc */
Memimage*
memlalloc(Memscreen *s, Rectangle screenr, Refreshfn refreshfn, void *refreshptr, rgba val)
{
    Memimage *n;
    Memlayer *l;
    /*s: [[memlalloc()]] other locals */
    static Memimage *paint;
    /*e: [[memlalloc()]] other locals */

    /*s: [[memlalloc()]] set paint once */
    if(paint == nil){
        paint = allocmemimage(Rect(0,0,1,1), RGBA32);
        /*s: [[memlalloc()]] sanity check paint */
        if(paint == nil)
            return nil;
        /*e: [[memlalloc()]] sanity check paint */
        paint->flags |= Frepl;
        paint->clipr = Rect(-0x3FFFFFF, -0x3FFFFFF, 0x3FFFFFF, 0x3FFFFFF);
    }
    /*e: [[memlalloc()]] set paint once */

    // share pixels data with screen image
    n = allocmemimaged(screenr, s->image->chan, s->image->data);
    /*s: [[memlalloc()]] sanity check n */
    if(n == nil)
        return nil;
    /*e: [[memlalloc()]] sanity check n */
    // overwrite zero and width derived from screenr in allocmemimaged
    // because the rectangle here does not match the data, it's a
    // subrectangle. Zero and width should remain the same.
    n->zero = s->image->zero;
    n->width = s->image->width;

    l = malloc(sizeof(Memlayer));
    /*s: [[memlalloc()]] sanity check l */
    if(l == nil){
        free(n);
        return nil;
    }
    /*e: [[memlalloc()]] sanity check l */

    l->screen = s;
    l->screenr = screenr;
    l->delta = Pt(0,0); // can be changed later by originwindow()
    l->clear = false;
    /*s: [[memlalloc()]] allocate save image */
    /*s: [[memlalloc()]] if refreshfn */
    if(refreshfn)
        l->save = nil;
    /*e: [[memlalloc()]] if refreshfn */
    else{
        l->save = allocmemimage(screenr, s->image->chan);
        /*s: [[memlalloc()]] sanity check l save */
        if(l->save == nil){
            free(l);
            free(n);
            return nil;
        }
        /*e: [[memlalloc()]] sanity check l save */
        /* allocmemimage doesn't initialize memory; this paints save area */
        if(val != DNofill)
            memfillcolor(l->save, val);
    }
    /*e: [[memlalloc()]] allocate save image */

    n->layer = l;
    // ok no more sanity check, we can make the connection
    n->data->ref++;

    /*s: [[memlalloc()]] set refresh fields part1 */
    l->refreshfn = refreshfn;
    l->refreshptr = nil;	/* don't set it until we're done */
    /*e: [[memlalloc()]] set refresh fields part1 */
    /*s: [[memlalloc()]] manage stack of windows */
    /* start with new window behind all existing ones */

    // add_end_double_list(l, s->frontmost, s->rearmost)
    l->front = s->rearmost;
    l->rear = nil;
    if(s->rearmost)
        s->rearmost->layer->rear = n;
    s->rearmost = n;
    if(s->frontmost == nil)
        s->frontmost = n;

    /* now pull new window to front */
    _memltofrontfill(n, val != DNofill);
    /*e: [[memlalloc()]] manage stack of windows */
    /*s: [[memlalloc()]] set refresh fields part2 */
    l->refreshptr = refreshptr;
    /*e: [[memlalloc()]] set refresh fields part2 */

    /*s: [[memlalloc()]] paint with requested color */
    /*
     * paint with requested color; previously exposed areas are already right
     * if this window has backing store, but just painting the whole thing is simplest.
     */
    if(val != DNofill){
        memsetchan(paint, n->chan);
        memfillcolor(paint, val);
        memdraw(n, n->r, paint, n->r.min, nil, n->r.min, S);
    }
    /*e: [[memlalloc()]] paint with requested color */
    return n;
}
/*e: function memlalloc */
/*e: lib_graphics/libmemlayer/lalloc.c */
