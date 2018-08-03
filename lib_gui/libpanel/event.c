/*s: lib_gui/libpanel/event.c */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

/*s: function [[plgrabkb]] */
void plgrabkb(Panel *g){
    plkbfocus=g;
}
/*e: function [[plgrabkb]] */
/*s: function [[plkeyboard]] */
void plkeyboard(Rune c){
    if(plkbfocus){
        // widget-specific callback
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
        else
            return g;
      }
    return nil;
}
/*e: function [[pl_ptinpanel]] */
/*s: function [[plmouse]] */
void plmouse(Panel *g, Mouse *m){
    Panel* hit;
    Panel* last = g->lastmouse;
    bool remouse;

    /*s: [[plmouse()]] if [[REMOUSE]] set [[hit]] to [[last]] */
    if(g->flags&REMOUSE)
        hit=last;
    /*e: [[plmouse()]] if [[REMOUSE]] set [[hit]] to [[last]] */
    else{
        hit=pl_ptinpanel(m->xy, g);
        if(last && last!=hit){
            /*s: [[plmouse()]] when [[last!=hit]] send [[OUT]] mouse event */
            m->buttons|=OUT;
            last->hit(last, m);
            m->buttons&=~OUT;
            /*e: [[plmouse()]] when [[last!=hit]] send [[OUT]] mouse event */
        }
    }
    if(hit){
        // widget-specific method
        remouse=hit->hit(hit, m);
        /*s: [[plmouse()]] handle [[remouse]] */
        if(remouse)
            g->flags|=REMOUSE;
        else
            g->flags&=~REMOUSE;
        /*e: [[plmouse()]] handle [[remouse]] */
        g->lastmouse=hit;
    }
    flushimage(display, true);
}
/*e: function [[plmouse]] */
/*e: lib_gui/libpanel/event.c */
