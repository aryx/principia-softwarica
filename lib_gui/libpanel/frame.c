/*s: lib_gui/libpanel/frame.c */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

/*s: function [[pl_drawframe]] */
void pl_drawframe(Panel *p){
    pl_box(p->b, p->r, FRAME);
}
/*e: function [[pl_drawframe]] */
/*s: function [[pl_hitframe]] */
bool pl_hitframe(Panel *p, Mouse *m){
    USED(p, m);
    return false;
}
/*e: function [[pl_hitframe]] */
/*s: function [[pl_typeframe]] */
void pl_typeframe(Panel *p, Rune c){
    USED(p, c);
}
/*e: function [[pl_typeframe]] */
/*s: function [[pl_getsizeframe]] */
Vector pl_getsizeframe(Panel *p, Vector children){
    USED(p);
    return pl_boxsize(children, FRAME);
}
/*e: function [[pl_getsizeframe]] */
/*s: function [[pl_childspaceframe]] */
void pl_childspaceframe(Panel *p, Point *ul, Vector *size){
    USED(p);
    pl_interior(FRAME, ul, size);
}
/*e: function [[pl_childspaceframe]] */
/*s: function [[plinitframe]] */
void plinitframe(Panel *v, int flags){
    v->flags=flags;

    v->draw=pl_drawframe;
    v->hit=pl_hitframe;
    v->type=pl_typeframe;

    v->getsize=pl_getsizeframe;
    v->childspace=pl_childspaceframe;

    v->kind="frame";
}
/*e: function [[plinitframe]] */
/*s: function [[plframe]] */
Panel *plframe(Panel *parent, int flags){
    Panel *p;

    p=pl_newpanel(parent, 0); // no widget-specific data
    plinitframe(p, flags);
    return p;
}
/*e: function [[plframe]] */
/*e: lib_gui/libpanel/frame.c */
