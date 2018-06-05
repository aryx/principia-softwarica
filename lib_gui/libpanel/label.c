/*s: lib_gui/libpanel/label.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"

typedef struct Label Label;
/*s: struct [[Label]] */
struct Label{
    // enum<Placement>
    int placement;
    // ref_own<Icon>
    Icon *icon;
};
/*e: struct [[Label]] */

/*s: function [[pl_drawlabel]] */
void pl_drawlabel(Panel *p){
    Label *l;
    l=p->data;
    pl_drawicon(p->b, pl_box(p->b, p->r, PASSIVE), l->placement, p->flags, l->icon);
}
/*e: function [[pl_drawlabel]] */
/*s: function [[pl_hitlabel]] */
int pl_hitlabel(Panel *p, Mouse *m){
    USED(p, m);
    return 0;
}
/*e: function [[pl_hitlabel]] */
/*s: function [[pl_typelabel]] */
void pl_typelabel(Panel *p, Rune c){
    USED(p, c);
}
/*e: function [[pl_typelabel]] */
/*s: function [[pl_getsizelabel]] */
Point pl_getsizelabel(Panel *p, Point children){
    USED(children);		/* shouldn't have any children */
    return pl_boxsize(pl_iconsize(p->flags, ((Label *)p->data)->icon), PASSIVE);
}
/*e: function [[pl_getsizelabel]] */
/*s: function [[pl_childspacelabel]] */
void pl_childspacelabel(Panel *g, Point *ul, Point *size){
    USED(g, ul, size);
}
/*e: function [[pl_childspacelabel]] */
/*s: function [[plinitlabel]] */
void plinitlabel(Panel *v, int flags, Icon *icon){
    v->flags=flags|LEAF;
    ((Label *)(v->data))->icon=icon;

    v->draw=pl_drawlabel;
    v->hit=pl_hitlabel;
    v->type=pl_typelabel;

    v->getsize=pl_getsizelabel;
    v->childspace=pl_childspacelabel;

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
    ((Label *)(p->data))->placement=placement;
}
/*e: function [[plplacelabel]] */
/*e: lib_gui/libpanel/label.c */
