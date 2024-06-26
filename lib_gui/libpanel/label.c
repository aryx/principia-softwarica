/*s: lib_gui/libpanel/label.c */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

typedef struct Label Label;

/*s: struct [[Label]] */
struct Label{
    // enum<Placement> (default = PLACECEN)
    int placement;
    // ref_own<Icon> = ref_own<Image|string> depending on Panel.flags&BITMAP
    Icon *icon;
};
/*e: struct [[Label]] */

/*s: function [[pl_drawlabel]] */
void pl_drawlabel(Panel *p){
    Label *l = p->data;

    pl_drawicon(p->b, pl_box(p->b, p->r, PASSIVE), 
                l->placement, p->flags, l->icon);
}
/*e: function [[pl_drawlabel]] */
/*s: function [[pl_hitlabel]] */
bool pl_hitlabel(Panel *p, Mouse *m){
    USED(p, m);
    return false;
}
/*e: function [[pl_hitlabel]] */
/*s: function [[pl_typelabel]] */
void pl_typelabel(Panel *p, Rune c){
    USED(p, c);
}
/*e: function [[pl_typelabel]] */
/*s: function [[pl_getsizelabel]] */
Vector pl_getsizelabel(Panel *p, Vector children){
    USED(children);		/* shouldn't have any children */
    return pl_boxsize(pl_iconsize(p->flags, ((Label *)p->data)->icon), 
                      PASSIVE);
}
/*e: function [[pl_getsizelabel]] */
/*s: function [[pl_childspacelabel]] */
void pl_childspacelabel(Panel *g, Point *ul, Vector *size){
    USED(g, ul, size);
}
/*e: function [[pl_childspacelabel]] */
/*s: function [[plinitlabel]] */
void plinitlabel(Panel *v, int flags, Icon *icon){
    Label* l = v->data;

    v->flags=flags|LEAF;

    v->draw=pl_drawlabel;
    v->hit=pl_hitlabel;
    v->type=pl_typelabel;

    v->getsize=pl_getsizelabel;
    v->childspace=pl_childspacelabel;

    l->icon=icon;
    //l->placement set in plplacelabel()

    v->kind="label";
}
/*e: function [[plinitlabel]] */
/*s: function [[pllabel]] */
Panel *pllabel(Panel *parent, int flags, Icon *icon){
    Panel *p;

    p=pl_newpanel(parent, sizeof(Label));
    plinitlabel(p, flags, icon);
    plplacelabel(p, PLACECEN);
    return p;
}
/*e: function [[pllabel]] */
/*s: function [[plplacelabel]] */
void plplacelabel(Panel *p, int placement){
    Label* l = p->data;

    l->placement=placement;
}
/*e: function [[plplacelabel]] */
/*e: lib_gui/libpanel/label.c */
