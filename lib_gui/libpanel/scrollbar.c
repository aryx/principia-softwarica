/*s: lib_gui/libpanel/scrollbar.c */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

typedef struct Scrollbar Scrollbar;
/*s: struct [[Scrollbar]] */
struct Scrollbar{
    int dir;		/* HORIZ or VERT */
    int lo, hi;		/* setting, in screen coordinates */
    int buttons;		/* saved mouse buttons for transmittal to scrollee */
    Rectangle interior;
    Point minsize;
};
/*e: struct [[Scrollbar]] */
/*s: constant [[SBWID]] */
#define	SBWID	15	/* should come from draw.c? */
/*e: constant [[SBWID]] */

/*s: function [[pl_drawscrollbar]] */
void pl_drawscrollbar(Panel *p){
    Scrollbar *sp;
    sp=p->data;
    sp->interior=pl_outline(p->b, p->r, p->state);
    pl_sliderupd(p->b, sp->interior, sp->dir, sp->lo, sp->hi);
}
/*e: function [[pl_drawscrollbar]] */
/*s: function [[pl_hitscrollbar]] */
int pl_hitscrollbar(Panel *g, Mouse *m){
    int oldstate, pos, len, dy;
    Point ul, size;
    Scrollbar *sp;
    sp=g->data;
    ul=g->r.min;
    size=subpt(g->r.max, g->r.min);
    pl_interior(g->state, &ul, &size);
    oldstate=g->state;
    if(!(g->flags & USERFL) && (m->buttons&OUT || !ptinrect(m->xy, g->r))){
        m->buttons&=~OUT;
        g->state=UP;
        goto out;
    }
    if(sp->dir==HORIZ){
        pos=m->xy.x-ul.x;
        len=size.x;
    }
    else{
        pos=m->xy.y-ul.y;
        len=size.y;
    }
    if(pos<0) pos=0;
    else if(pos>len) pos=len;
    if(m->buttons&7){
        g->state=DOWN;
        sp->buttons=m->buttons;
        switch(m->buttons){
        case 1:
            dy=pos*(sp->hi-sp->lo)/len;
            pl_sliderupd(g->b, sp->interior, sp->dir, sp->lo-dy,
                sp->hi-dy);
            break;
        case 2:
            if(g->scrollee && g->scrollee->scroll)
                g->scrollee->scroll(g->scrollee, sp->dir,
                    m->buttons, pos, len);
            break;
        case 4:
            dy=pos*(sp->hi-sp->lo)/len;
            pl_sliderupd(g->b, sp->interior, sp->dir, sp->lo+dy,
                sp->hi+dy);
            break;
        }
    }
    else{
        if(!(sp->buttons&2) && g->state==DOWN && g->scrollee && g->scrollee->scroll)
            g->scrollee->scroll(g->scrollee, sp->dir, sp->buttons,
                pos, len);
        g->state=UP;
    }
out:
    if(oldstate!=g->state) pldraw(g, g->b);
    return g->state==DOWN;
}
/*e: function [[pl_hitscrollbar]] */
/*s: function [[pl_typescrollbar]] */
void pl_typescrollbar(Panel *p, Rune c){
    USED(p, c);
}
/*e: function [[pl_typescrollbar]] */
/*s: function [[pl_getsizescrollbar]] */
Point pl_getsizescrollbar(Panel *p, Point children){
    USED(children);
    return pl_boxsize(((Scrollbar *)p->data)->minsize, p->state);
}
/*e: function [[pl_getsizescrollbar]] */
/*s: function [[pl_childspacescrollbar]] */
void pl_childspacescrollbar(Panel *p, Point *ul, Point *size){
    USED(p, ul, size);
}
/*e: function [[pl_childspacescrollbar]] */
/*s: function [[pl_setscrollbarscrollbar]] */
/*
 * Arguments lo, hi and len are in the scrollee's natural coordinates
 */
void pl_setscrollbarscrollbar(Panel *p, int lo, int hi, int len){
    Point ul, size;
    int mylen;
    Scrollbar *sp;
    sp=p->data;
    ul=p->r.min;
    size=subpt(p->r.max, p->r.min);
    pl_interior(p->state, &ul, &size);
    mylen=sp->dir==HORIZ?size.x:size.y;
    if(len==0) len=1;
    sp->lo=lo*mylen/len;
    sp->hi=hi*mylen/len;
    if(sp->lo<0) sp->lo=0;
    if(sp->lo>=mylen) sp->hi=mylen-1;
    if(sp->hi<=sp->lo) sp->hi=sp->lo+1;
    if(sp->hi>mylen) sp->hi=mylen;
    pldraw(p, p->b);
}
/*e: function [[pl_setscrollbarscrollbar]] */
/*s: function [[pl_priscrollbar]] */
int pl_priscrollbar(Panel *, Point){
    return PRI_SCROLLBAR;
}
/*e: function [[pl_priscrollbar]] */
/*s: function [[plinitscrollbar]] */
void plinitscrollbar(Panel *v, int flags){
    Scrollbar *sp;
    sp=v->data;
    v->flags=flags|LEAF;
    v->pri=pl_priscrollbar;
    v->state=UP;
    v->draw=pl_drawscrollbar;
    v->hit=pl_hitscrollbar;
    v->type=pl_typescrollbar;
    v->getsize=pl_getsizescrollbar;
    v->childspace=pl_childspacescrollbar;
    v->setscrollbar=pl_setscrollbarscrollbar;
    switch(flags&PACK){
    case PACKN:
    case PACKS:
        sp->dir=HORIZ;
        sp->minsize=Pt(0, SBWID);
        v->flags|=FILLX;
        break;
    case PACKE:
    case PACKW:
        sp->dir=VERT;
        sp->minsize=Pt(SBWID, 0);
        v->flags|=FILLY;
        break;
    }
    sp->lo=0;
    sp->hi=0;
    v->kind="scrollbar";
}
/*e: function [[plinitscrollbar]] */
/*s: function [[plscrollbar]] */
Panel *plscrollbar(Panel *parent, int flags){
    Panel *v;
    v=pl_newpanel(parent, sizeof(Scrollbar));
    plinitscrollbar(v, flags);
    return v;
}
/*e: function [[plscrollbar]] */
/*e: lib_gui/libpanel/scrollbar.c */
