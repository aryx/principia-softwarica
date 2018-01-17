/*s: windows/libframe/frinit.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include <mouse.h>
#include <frame.h>

/*s: function [[frinit]] */
void
frinit(Frame *f, Rectangle r, Font *ft, Image *b, Image *cols[NCOL])
{
    frsetrects(f, r, b);
    f->font = ft;
    f->display = b->display;

    f->nchars = 0;
    f->nlines = 0;
    /*s: [[frinit()]] initialize other fields */
    if(cols != nil)
        memmove(f->cols, cols, sizeof f->cols);
    /*x: [[frinit()]] initialize other fields */
    if(f->tick == nil && f->cols[BACK] != nil)
        frinittick(f);
    /*x: [[frinit()]] initialize other fields */
    f->box = nil;
    f->nbox = 0;
    f->nalloc = 0;
    /*x: [[frinit()]] initialize other fields */
    f->p0 = 0;
    f->p1 = 0;
    /*x: [[frinit()]] initialize other fields */
    f->lastlinefull = 0;
    /*x: [[frinit()]] initialize other fields */
    f->maxtab = 8 * stringwidth(ft, "0");
    /*e: [[frinit()]] initialize other fields */
}
/*e: function [[frinit]] */

/*s: function [[frinittick]] */
void
frinittick(Frame *f)
{
    Image *b = f->display->screenimage;
    Font *ft = f->font;

    /*s: [[frinittick()]] free old tick */
    if(f->tick)
        freeimage(f->tick);
    /*e: [[frinittick()]] free old tick */
    f->tick = allocimage(f->display, Rect(0, 0, FRTICKW, ft->height), b->chan, 0, DWhite);
    /*s: [[frinittick()]] sanity check tick */
    if(f->tick == nil)
        return;
    /*e: [[frinittick()]] sanity check tick */
    /*s: [[frinittick()]] free old tickback */
    if(f->tickback)
        freeimage(f->tickback);
    /*e: [[frinittick()]] free old tickback */
    f->tickback = allocimage(f->display, f->tick->r, b->chan, false, DWhite);
    /*s: [[frinittick()]] sanity check tickback */
    if(f->tickback == nil){
        freeimage(f->tick);
        f->tick = nil;
        return;
    }
    /*e: [[frinittick()]] sanity check tickback */
    /* background color */
    draw(f->tick, f->tick->r, f->cols[BACK], nil, ZP);
    /* vertical line */
    draw(f->tick, Rect(FRTICKW/2, 0, FRTICKW/2+1, ft->height), f->cols[TEXT], nil, ZP);
    /* box on each end */
    draw(f->tick, Rect(0, 0, FRTICKW, FRTICKW), f->cols[TEXT], nil, ZP);
    draw(f->tick, Rect(0, ft->height-FRTICKW, FRTICKW, ft->height), f->cols[TEXT], nil, ZP);
}
/*e: function [[frinittick]] */

/*s: function [[frsetrects]] */
void
frsetrects(Frame *f, Rectangle r, Image *b)
{
    f->b = b;
    f->entire = r;
    f->r = r;
    f->r.max.y -= (r.max.y-r.min.y) % f->font->height;
    f->maxlines = (r.max.y-r.min.y) / f->font->height;
}
/*e: function [[frsetrects]] */

/*s: function [[frclear]] */
void
frclear(Frame *f, bool freeall)
{
    /*s: [[frclear()]] free boxes */
    if(f->nbox)
        _frdelbox(f, 0, f->nbox-1);
    if(f->box)
        free(f->box);
    f->box = nil;
    /*e: [[frclear()]] free boxes */
    /*s: [[frclear()]] free ticks */
    if(freeall){
        freeimage(f->tick);
        freeimage(f->tickback);
        f->tick = nil;
        f->tickback = nil;
    }
    f->ticked = false;
    /*e: [[frclear()]] free ticks */
}
/*e: function [[frclear]] */
/*e: windows/libframe/frinit.c */
