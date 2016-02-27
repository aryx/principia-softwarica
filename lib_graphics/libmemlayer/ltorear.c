/*s: lib_graphics/libmemlayer/ltorear.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <memlayer.h>

/*s: function _memltorear */
void
_memltorear(Memimage *i, Memimage *rear)
{
    Memlayer *l;
    Memscreen *s;
    Memimage *f, *r, *rr;
    Rectangle x;
    bool overlap;

    l = i->layer;
    s = l->screen;

    while(l->rear != rear){
        r = l->rear;
        x = l->screenr;

        overlap = rectclip(&x, r->layer->screenr);
        if(overlap){
            memlhide(i, x);
            l->clear = false;
        }
        /*s: [[_memltorear()]] put i behind r */
        /* swap l and r in screen's list */
        // swap_end_double_list(i, r, s->frontmost, s->rearmost)
        rr = r->layer->rear;
        f = l->front;

        if(rr == nil)
            s->rearmost = i;
        else
            rr->layer->front = i;

        if(f == nil)
            s->frontmost = r;
        else
            f->layer->rear = r;

        l->rear = rr;
        l->front = r; // i is now behind r
        r->layer->rear = i;
        r->layer->front = f;
        /*e: [[_memltorear()]] put i behind r */
        if(overlap)
            memlexpose(r, x);
    }
}
/*e: function _memltorear */

/*s: function memltorear */
void
memltorear(Memimage *i)
{
    _memltorear(i, nil);
    _memlsetclear(i->layer->screen);
}
/*e: function memltorear */

/*s: function memltorearn */
void
memltorearn(Memimage **ip, int n)
{
    Memimage *i, *rear;

    /*s: [[memltofrontn()]] sanity check n */
    if(n == 0)
        return;
    /*e: [[memltofrontn()]] sanity check n */
    rear = nil;
    while(--n >= 0){
        i = *ip++;
        _memltorear(i, rear);
        rear = i;
    }
    _memlsetclear(rear->layer->screen);
}
/*e: function memltorearn */
/*e: lib_graphics/libmemlayer/ltorear.c */
