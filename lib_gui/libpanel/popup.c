/*s: lib_gui/libpanel/popup.c */
/*
 * popup
 *	looks like a group, except diverts hits on certain buttons to
 *	panels that it temporarily pops up.
 */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

typedef struct Popup Popup;

/*s: struct [[Popup]] */
struct Popup{
    Panel *pop[3];			/* what to pop up */

    //option<ref_own<Image>>
    Image *save;			/* where to save what the popup covers */
};
/*e: struct [[Popup]] */

/*s: function [[pl_drawpopup]] */
void pl_drawpopup(Panel *p){
    USED(p);
}
/*e: function [[pl_drawpopup]] */
/*s: function [[pl_hitpopup]] */
bool pl_hitpopup(Panel *g, Mouse *m){
    Panel *p;
    Popup *pp = g->data;
    /*s: [[pl_hitpopup()]] other locals */
    Vector d; // delta
    /*e: [[pl_hitpopup()]] other locals */

    if(g->state==UP){
        switch(m->buttons&7){
        case CLICK_LEFT:   p=pp->pop[0]; g->state=DOWN1; break;
        case CLICK_MIDDLE: p=pp->pop[1]; g->state=DOWN2; break;
        case CLICK_RIGHT:  p=pp->pop[2]; g->state=DOWN3; break;
        /*s: [[pl_hitpopup()]] when [[UP]], switch buttons other cases */
        // just a mouse motion
        case 0: p=g->child; break;
        /*x: [[pl_hitpopup()]] when [[UP]], switch buttons other cases */
        default: p=nil; break;
        /*e: [[pl_hitpopup()]] when [[UP]], switch buttons other cases */
        }
        /*s: [[pl_hitpopup()]] when [[UP]], if no popup menu [[p]] */
        if(p==nil){
            p=g->child;
            g->state=DOWN;
        }
        /*e: [[pl_hitpopup()]] when [[UP]], if no popup menu [[p]] */
        else if(g->state!=UP){
            /*s: [[pl_hitpopup()]] when from [[UP]] to [[DOWNX]] and valid [[p]] */
            plpack(p, view->clipr);
            /*s: [[pl_hitpopup()]] when from [[UP]] to [[DOWNX]], compute [[d]] */
            if(p->lastmouse)
                d=subpt(m->xy, divpt(addpt(p->lastmouse->r.min,
                             p->lastmouse->r.max), 2));
            else
                d=subpt(m->xy, divpt(addpt(p->r.min, p->r.max), 2));
            if(p->r.min.x+d.x<g->r.min.x) d.x=g->r.min.x-p->r.min.x;
            if(p->r.max.x+d.x>g->r.max.x) d.x=g->r.max.x-p->r.max.x;
            if(p->r.min.y+d.y<g->r.min.y) d.y=g->r.min.y-p->r.min.y;
            if(p->r.max.y+d.y>g->r.max.y) d.y=g->r.max.y-p->r.max.y;
            /*e: [[pl_hitpopup()]] when from [[UP]] to [[DOWNX]], compute [[d]] */
            plmove(p, d);
            pp->save=allocimage(display, p->r, g->b->chan, false, DNofill);
            if(pp->save!=nil) 
                draw(pp->save, p->r, g->b, nil, p->r.min);
            pl_invis(p, false);
            pldraw(p, g->b);
            /*e: [[pl_hitpopup()]] when from [[UP]] to [[DOWNX]] and valid [[p]] */
        }
    } else{ // a DOWNX state
        switch(g->state){
        case DOWN1: p=pp->pop[0]; break;
        case DOWN2: p=pp->pop[1]; break;
        case DOWN3: p=pp->pop[2]; break;
        case DOWN:  p=g->child;  break;
        default: SET(p); break;			/* can't happen! */
        }
        /*s: [[pl_hitpopup()]] when [[DOWNX]] state, if no buttons */
        if((m->buttons&7)==0){
            if(g->state!=DOWN){
                if(pp->save!=nil){
                    // restore from saved image
                    draw(g->b, p->r, pp->save, nil, p->r.min);
                    flushimage(display, true);
                    freeimage(pp->save);
                    pp->save=nil;
                }
                pl_invis(p, true);
            }
            g->state=UP;
        }
        /*e: [[pl_hitpopup()]] when [[DOWNX]] state, if no buttons */
    }
    // redispatch mouse event to appropriate menu or child
    plmouse(p, m);

    if((m->buttons&7)==0)
        g->state=UP;
    return (m->buttons&7)!=0;
}
/*e: function [[pl_hitpopup]] */
/*s: function [[pl_typepopup]] */
void pl_typepopup(Panel *g, Rune c){
    USED(g, c);
}
/*e: function [[pl_typepopup]] */
/*s: function [[pl_getsizepopup]] */
Vector pl_getsizepopup(Panel *g, Vector children){
    USED(g);
    return children;
}
/*e: function [[pl_getsizepopup]] */
/*s: function [[pl_childspacepopup]] */
void pl_childspacepopup(Panel *g, Point *ul, Vector *size){
    USED(g, ul, size);
}
/*e: function [[pl_childspacepopup]] */
/*s: function [[pl_pripopup]] */
int pl_pripopup(Panel *, Point){
    return PRI_POPUP;
}
/*e: function [[pl_pripopup]] */
/*s: function [[plinitpopup]] */
void plinitpopup(Panel *v, int flags, Panel *pop0, Panel *pop1, Panel *pop2){
    Popup *pp =v->data;

    v->flags=flags;
    v->state=UP;

    v->draw=pl_drawpopup;
    v->hit=pl_hitpopup;
    v->type=pl_typepopup;

    v->getsize=pl_getsizepopup;
    v->childspace=pl_childspacepopup;

    v->pri=pl_pripopup;

    pp->pop[0]=pop0;
    pp->pop[1]=pop1;
    pp->pop[2]=pop2;
    pp->save=nil;

    v->kind="popup";
}
/*e: function [[plinitpopup]] */
/*s: function [[plpopup]] */
Panel *plpopup(Panel *parent, int flags, Panel *pop0, Panel *pop1, Panel *pop2){
    Panel *v;

    v=pl_newpanel(parent, sizeof(Popup));
    plinitpopup(v, flags, pop0, pop1, pop2);
    return v;
}
/*e: function [[plpopup]] */
/*e: lib_gui/libpanel/popup.c */
