/*s: lib_gui/libpanel/scroll.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"
/*s: function plscroll */
void plscroll(Panel *scrollee, Panel *xscroller, Panel *yscroller){
    scrollee->xscroller=xscroller;
    scrollee->yscroller=yscroller;
    if(xscroller) xscroller->scrollee=scrollee;
    if(yscroller) yscroller->scrollee=scrollee;
}
/*e: function plscroll */
/*s: function plgetscroll */
Scroll plgetscroll(Panel *p){
    return p->scr;
}
/*e: function plgetscroll */
/*s: function plsetscroll */
void plsetscroll(Panel *p, Scroll s){
    if(p->scroll){
        if(s.size.x) p->scroll(p, HORIZ, 2, s.pos.x, s.size.x);
        if(s.size.y) p->scroll(p, VERT, 2, s.pos.y, s.size.y);
    }
}
/*e: function plsetscroll */
/*e: lib_gui/libpanel/scroll.c */
