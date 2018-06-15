/*s: lib_gui/libpanel/pulldown.c */
/*
 * pulldown
 *	makes a button that pops up a panel when hit
 */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

typedef struct Pulldown Pulldown;

/*s: struct [[Pulldown]] */
struct Pulldown{
    Icon *icon;		/* button label */
    Panel *pull;		/* Panel to pull down */
    int side;		/* which side of the button to put the panel on */
    Image *save;		/* where to save what we draw the panel on */
};
/*e: struct [[Pulldown]] */

/*s: function [[pl_drawpulldown]] */
void pl_drawpulldown(Panel *p){
    pl_drawicon(p->b, pl_box(p->b, p->r, p->state), PLACECEN,
        p->flags, ((Pulldown *)p->data)->icon);
}
/*e: function [[pl_drawpulldown]] */
/*s: function [[pl_hitpulldown]] */
int pl_hitpulldown(Panel *g, Mouse *m){
    int oldstate, passon;
    Rectangle r;
    Panel *p, *hitme;
    Pulldown *pp;
    pp=g->data;
    oldstate=g->state;
    p=pp->pull;
    hitme=0;
    switch(g->state){
    case UP:
        if(!ptinrect(m->xy, g->r))
            g->state=UP;
        else if(m->buttons&7){
            r=g->b->r;
            p->flags&=~PLACE;
            switch(pp->side){
            case PACKN:
                r.min.x=g->r.min.x;
                r.max.y=g->r.min.y;
                p->flags|=PLACESW;
                break;
            case PACKS:
                r.min.x=g->r.min.x;
                r.min.y=g->r.max.y;
                p->flags|=PLACENW;
                break;
            case PACKE:
                r.min.x=g->r.max.x;
                r.min.y=g->r.min.y;
                p->flags|=PLACENW;
                break;
            case PACKW:
                r.max.x=g->r.min.x;
                r.min.y=g->r.min.y;
                p->flags|=PLACENE;
                break;
            case PACKCEN:
                r.min=g->r.min;
                p->flags|=PLACENW;
                break;
            }
            plpack(p, r);
            pp->save=allocimage(display, p->r, g->b->chan, 0, DNofill);
            if(pp->save!=0) draw(pp->save, p->r, g->b, 0, p->r.min);
            pl_invis(p, 0);
            pldraw(p, g->b);
            g->state=DOWN;
        }
        break;
    case DOWN:
        if(!ptinrect(m->xy, g->r)){
            switch(pp->side){
            default: SET(passon); break;		/* doesn't happen */
            case PACKN: passon=m->xy.y<g->r.min.y; break;
            case PACKS: passon=m->xy.y>=g->r.max.y; break;
            case PACKE: passon=m->xy.x>=g->r.max.x; break;
            case PACKW: passon=m->xy.x<g->r.min.x; break;
            case PACKCEN: passon=1; break;
            }
            if(passon){
                hitme=p;
                if((m->buttons&7)==0) g->state=UP;
            }
            else	g->state=UP;
        }
        else if((m->buttons&7)==0) g->state=UP;
        else hitme=p;
        if(g->state!=DOWN && pp->save){
            draw(g->b, p->r, pp->save, 0, p->r.min);
            freeimage(pp->save);
            pp->save=0;
            pl_invis(p, 1);
            hitme=p;
        }
    }
    if(g->state!=oldstate) pldraw(g, g->b);
    if(hitme) plmouse(hitme, m);
    return g->state==DOWN;
}
/*e: function [[pl_hitpulldown]] */
/*s: function [[pl_typepulldown]] */
void pl_typepulldown(Panel *p, Rune c){
    USED(p, c);
}
/*e: function [[pl_typepulldown]] */
/*s: function [[pl_getsizepulldown]] */
Point pl_getsizepulldown(Panel *p, Point children){
    USED(p, children);
    return pl_boxsize(pl_iconsize(p->flags, ((Pulldown *)p->data)->icon), p->state);
}
/*e: function [[pl_getsizepulldown]] */
/*s: function [[pl_childspacepulldown]] */
void pl_childspacepulldown(Panel *p, Point *ul, Point *size){
    USED(p, ul, size);
}
/*e: function [[pl_childspacepulldown]] */
/*s: function [[plinitpulldown]] */
void plinitpulldown(Panel *v, int flags, Icon *icon, Panel *pullthis, int side){
    Pulldown *pp;
    pp=v->data;
    v->flags=flags|LEAF;
    v->draw=pl_drawpulldown;
    v->hit=pl_hitpulldown;
    v->type=pl_typepulldown;
    v->getsize=pl_getsizepulldown;
    v->childspace=pl_childspacepulldown;
    pp->pull=pullthis;
    pp->side=side;
    pp->icon=icon;
    v->kind="pulldown";
}
/*e: function [[plinitpulldown]] */
/*s: function [[plpulldown]] */
Panel *plpulldown(Panel *parent, int flags, Icon *icon, Panel *pullthis, int side){
    Panel *v;

    v=pl_newpanel(parent, sizeof(Pulldown));
    v->state=UP;
    ((Pulldown *)v->data)->save=0;
    plinitpulldown(v, flags, icon, pullthis, side);
    return v;
}
/*e: function [[plpulldown]] */
/*s: function [[plmenubar]] */
Panel *plmenubar(Panel *parent, int flags, int cflags, Icon *l1, Panel *m1, Icon *l2, ...){
    Panel *v;
    va_list arg;
    Icon *s;
    int pulldir;

    switch(cflags&PACK){
    default:
        SET(pulldir);
        break;
    case PACKE:
    case PACKW:
        pulldir=PACKS;
        break;
    case PACKN:
    case PACKS:
        pulldir=PACKE;
        break;
    }
    v=plgroup(parent, flags);
    va_start(arg, cflags);
    while((s=va_arg(arg, Icon *))!=0)
        plpulldown(v, cflags, s, va_arg(arg, Panel *), pulldir);
    va_end(arg);
    USED(l1, m1, l2);
    v->kind="menubar";
    return v;
}
/*e: function [[plmenubar]] */
/*e: lib_gui/libpanel/pulldown.c */
