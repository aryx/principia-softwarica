/*s: lib_gui/libpanel/event.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"

/*s: function [[plgrabkb]] */
void plgrabkb(Panel *g){
    plkbfocus=g;
}
/*e: function [[plgrabkb]] */
/*s: function [[plkeyboard]] */
void plkeyboard(Rune c){
    if(plkbfocus){
        plkbfocus->type(plkbfocus, c);
        flushimage(display, true);
    }
}
/*e: function [[plkeyboard]] */

/*s: function [[pl_ptinpanel]] */
/*
 * Return the most leafward, highest priority panel containing p
 */
Panel *pl_ptinpanel(Point p, Panel *g){
    Panel *v;
    for(;g;g=g->next) 
      if(ptinrect(p, g->r)){
        //recurse
        v=pl_ptinpanel(p, g->child);
        if(v && v->pri(v, p) >= g->pri(g, p)) 
            return v;
        return g;
      }
    return 0;
}
/*e: function [[pl_ptinpanel]] */
/*s: function [[plmouse]] */
void plmouse(Panel *g, Mouse *m){
    Panel *hit, *last;

    if(g->flags&REMOUSE)
        hit=g->lastmouse;
    else{

        hit=pl_ptinpanel(m->xy, g);

        last=g->lastmouse;
        if(last && last!=hit){
            m->buttons|=OUT;
            last->hit(last, m);
            m->buttons&=~OUT;
        }
    }
    if(hit){
        if(hit->hit(hit, m))
            g->flags|=REMOUSE;
        else
            g->flags&=~REMOUSE;
        g->lastmouse=hit;
    }
    flushimage(display, 1);
}
/*e: function [[plmouse]] */
/*e: lib_gui/libpanel/event.c */
