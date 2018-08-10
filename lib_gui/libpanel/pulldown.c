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

    // enum<PackingDirection>
    int side;		/* which side of the button to put the panel on */
    //option<ref_own<Image>>
    Image *save;		/* where to save what we draw the panel on */
};
/*e: struct [[Pulldown]] */

/*s: function [[pl_drawpulldown]] */
void pl_drawpulldown(Panel *p){
    Pulldown* pd = p->data;

    pl_drawicon(p->b, pl_box(p->b, p->r, p->state), PLACECEN,
        p->flags, pd->icon);
}
/*e: function [[pl_drawpulldown]] */
/*s: function [[pl_hitpulldown]] */
bool pl_hitpulldown(Panel *g, Mouse *m){
    Pulldown *pp = g->data;
    Panel *p = pp->pull;
    Panel *hitme = nil;
    int oldstate;
    Rectangle r;
    bool passon;

    oldstate=g->state;

    switch(g->state){
    case UP:
        /*s: [[pl_hitpulldown()]] when [[UP]], if mouse outside widget */
        if(!ptinrect(m->xy, g->r))
            g->state=UP;
        /*e: [[pl_hitpulldown()]] when [[UP]], if mouse outside widget */
        else 
            if(m->buttons&7){
                r=g->b->r;
                p->flags&=~PLACE;
                switch(pp->side){
                /*s: [[pl_hitpulldown()]] when [[UP]] and buttons, switch side cases */
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
                /*e: [[pl_hitpulldown()]] when [[UP]] and buttons, switch side cases */
                }
                plpack(p, r);
                pp->save=allocimage(display, p->r, g->b->chan, false, DNofill);
                if(pp->save!=nil) 
                    draw(pp->save, p->r, g->b, nil, p->r.min);
                pl_invis(p, false);
                pldraw(p, g->b);
                g->state=DOWN;
             }
        break;
    case DOWN:
        if(!ptinrect(m->xy, g->r)){
            switch(pp->side){
            case PACKN: passon=m->xy.y<g->r.min.y; break;
            case PACKS: passon=m->xy.y>=g->r.max.y; break;
            case PACKE: passon=m->xy.x>=g->r.max.x; break;
            case PACKW: passon=m->xy.x<g->r.min.x; break;
            case PACKCEN: passon=true; break;
            default: SET(passon); break;		/* doesn't happen */
            }
            if(passon){
                hitme=p;
                if((m->buttons&7)==0) 
                    g->state=UP;
            }
            else	g->state=UP;
        }
        else if((m->buttons&7)==0) g->state=UP;
        else hitme=p;

        if(g->state!=DOWN && pp->save){
            draw(g->b, p->r, pp->save, nil, p->r.min);
            freeimage(pp->save);
            pp->save=nil;
            pl_invis(p, true);
            hitme=p;
        }
    }
    if(g->state!=oldstate) 
        pldraw(g, g->b);
    if(hitme) 
        plmouse(hitme, m);

    return g->state==DOWN;
}
/*e: function [[pl_hitpulldown]] */
/*s: function [[pl_typepulldown]] */
void pl_typepulldown(Panel *p, Rune c){
    USED(p, c);
}
/*e: function [[pl_typepulldown]] */
/*s: function [[pl_getsizepulldown]] */
Vector pl_getsizepulldown(Panel *p, Vector children){
    USED(p, children);
    return pl_boxsize(pl_iconsize(p->flags, ((Pulldown *)p->data)->icon), p->state);
}
/*e: function [[pl_getsizepulldown]] */
/*s: function [[pl_childspacepulldown]] */
void pl_childspacepulldown(Panel *p, Point *ul, Vector *size){
    USED(p, ul, size);
}
/*e: function [[pl_childspacepulldown]] */
/*s: function [[plinitpulldown]] */
void plinitpulldown(Panel *v, int flags, Icon *icon, Panel *pullthis, int side){
    Pulldown *pp = v->data;

    v->flags=flags|LEAF;
    v->state=UP;

    v->draw=pl_drawpulldown;
    v->hit=pl_hitpulldown;
    v->type=pl_typepulldown;

    v->getsize=pl_getsizepulldown;
    v->childspace=pl_childspacepulldown;

    pp->icon=icon;
    pp->pull=pullthis;
    pp->side=side;
    pp->save=nil;

    v->kind="pulldown";
}
/*e: function [[plinitpulldown]] */
/*s: function [[plpulldown]] */
Panel *plpulldown(Panel *parent, int flags, Icon *icon, Panel *pullthis, int side){
    Panel *v;

    v=pl_newpanel(parent, sizeof(Pulldown));
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

    /*s: [[plmenubar()]] set [[pulldir]] based on [[cflags]] */
    switch(cflags&PACK){
    case PACKE:
    case PACKW:
        pulldir=PACKS;
        break;
    case PACKN:
    case PACKS:
        pulldir=PACKE;
        break;
    default:
        SET(pulldir);
        break;
    }
    /*e: [[plmenubar()]] set [[pulldir]] based on [[cflags]] */
    v=plgroup(parent, flags);

    va_start(arg, cflags);
    while((s=va_arg(arg, Icon *))!=nil)
        plpulldown(v, cflags, s, va_arg(arg, Panel *), pulldir);
    va_end(arg);

    USED(l1, m1, l2); // used for type checking at least the first arg
    v->kind="menubar";
    return v;
}
/*e: function [[plmenubar]] */
/*e: lib_gui/libpanel/pulldown.c */
