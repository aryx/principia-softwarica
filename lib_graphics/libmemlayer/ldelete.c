/*s: lib_graphics/libmemlayer/ldelete.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <memlayer.h>

/*s: function memldelete */
void
memldelete(Memimage *i)
{
    Memlayer *l;
    Memscreen *s;

    l = i->layer;
    s = i->layer->screen;

    /* free backing store and disconnect refresh, to make pushback fast */
    freememimage(l->save);
    l->save = nil;
    l->refreshptr = nil;
    memltorear(i);

    /* window is now the rearmost;  clean up screen structures and deallocate */

    if(s->fill){
        i->clipr = i->r;
        memdraw(i, i->r, s->fill, i->r.min, nil, i->r.min, S);
    }
    /*s: [[memldelete()]] manage stack of windows */
    // remove_double_list(l, s->frontmost, s->rearmost)
    if(l->front){
        l->front->layer->rear = nil;
        s->rearmost = l->front;
    }else{
        s->frontmost = nil;
        s->rearmost = nil;
    }
    /*e: [[memldelete()]] manage stack of windows */

    free(l);
    freememimage(i);
}
/*e: function memldelete */

/*s: function memlfree */
/*
 * Just free the data structures, don't do graphics
 */
void
memlfree(Memimage *i)
{
    Memlayer *l;

    l = i->layer;
    freememimage(l->save);
    free(l);
    freememimage(i);
}
/*e: function memlfree */

/*s: function _memlsetclear */
void
_memlsetclear(Memscreen *s)
{
    Memimage *i, *j;
    Memlayer *l;

    for(i = s->rearmost; i; i = i->layer->front){
        l = i->layer;
        l->clear = rectinrect(l->screenr, l->screen->image->clipr);
        if(l->clear)
            for(j = l->front; j; j = j->layer->front)
                if(rectXrect(l->screenr, j->layer->screenr)){
                    l->clear = false;
                    break;
                }
    }
}
/*e: function _memlsetclear */
/*e: lib_graphics/libmemlayer/ldelete.c */
