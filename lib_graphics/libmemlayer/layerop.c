/*s: lib_graphics/libmemlayer/layerop.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <memlayer.h>

/*s: function RECUR */
#define	RECUR(a,b,c,d)	_layerop(fn, i, Rect(a, b, c, d), clipr, etc, front->layer->rear);
/*e: function RECUR */

/*s: function _layerop */
static void
_layerop(
    void (*fn)(Memimage*, Rectangle, Rectangle, void*, int),
    Memimage *i,
    Rectangle r,
    Rectangle clipr,
    void *etc,
    Memimage *front)
{
    Rectangle fr;

    Top:
    if(front == i){
        /* no one is in front of this part of window; use the screen */
        fn(i->layer->screen->image, r, clipr, etc, false);
        return;
    }
    // else

    fr = front->layer->screenr;
    if(!rectXrect(r, fr)){
        /* r doesn't touch this window; continue on next rearmost */
        front = front->layer->rear;
        goto Top;
    }
    // else, r touches this window somewhere

    if(fr.max.y < r.max.y){
        // rectangle below front
        RECUR(r.min.x, fr.max.y, r.max.x, r.max.y);
        r.max.y = fr.max.y;
    }
    if(r.min.y < fr.min.y){
        // rectangle above front
        RECUR(r.min.x, r.min.y, r.max.x, fr.min.y);
        r.min.y = fr.min.y;
    }
    if(fr.max.x < r.max.x){
        // rectangle right of front
        RECUR(fr.max.x, r.min.y, r.max.x, r.max.y);
        r.max.x = fr.max.x;
    }
    if(r.min.x < fr.min.x){
        // rectangle left of front
        RECUR(r.min.x, r.min.y, fr.min.x, r.max.y);
        r.min.x = fr.min.x;
    }
    /* r is covered by front, so put in save area */
    (*fn)(i->layer->save, r, clipr, etc, true);
}
/*e: function _layerop */

/*s: function _memlayerop */
/*
 * Assumes incoming rectangle has already been clipped to i's logical r and clipr
 */
void
_memlayerop(
    void (*fn)(Memimage*, Rectangle, Rectangle, void*, int),
    Memimage *i,
    Rectangle screenr,	/* clipped to window boundaries */
    Rectangle clipr,	/* clipped also to clipping rectangles of hierarchy */
    void *etc)
{
    Memlayer *l;
    Rectangle r, scr;

    l = i->layer;
    if(!rectclip(&screenr, l->screenr))
        return;

    if(l->clear){
        fn(l->screen->image, screenr, clipr, etc, false);
        return;
    }

    r = screenr; // original value before rectclip below
    scr = l->screen->image->clipr;

    /*
     * Do the piece on the screen
     */
    if(rectclip(&screenr, scr))
        _layerop(fn, i, screenr, clipr, etc, l->screen->frontmost);

    if(rectinrect(r, scr))
        return;

    /*
     * Do the piece off the screen
    */
    if(!rectXrect(r, scr)){
        /* completely offscreen; easy */
        fn(l->save, r, clipr, etc, true);
        return;
    }
    // else

    if(r.min.y < scr.min.y){
        /* above screen */
        fn(l->save, Rect(r.min.x, r.min.y, r.max.x, scr.min.y), clipr, etc, true);
        r.min.y = scr.min.y;
    }
    if(r.max.y > scr.max.y){
        /* below screen */
        fn(l->save, Rect(r.min.x, scr.max.y, r.max.x, r.max.y), clipr, etc, true);
        r.max.y = scr.max.y;
    }
    if(r.min.x < scr.min.x){
        /* left of screen */
        fn(l->save, Rect(r.min.x, r.min.y, scr.min.x, r.max.y), clipr, etc, true);
        r.min.x = scr.min.x;
    }
    if(r.max.x > scr.max.x){
        /* right of screen */
        fn(l->save, Rect(scr.max.x, r.min.y, r.max.x, r.max.y), clipr, etc, true);
    }
}
/*e: function _memlayerop */
/*e: lib_graphics/libmemlayer/layerop.c */
