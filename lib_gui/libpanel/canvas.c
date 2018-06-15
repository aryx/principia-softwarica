/*s: lib_gui/libpanel/canvas.c */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

typedef struct Canvas Canvas;

/*s: struct [[Canvas]] */
struct Canvas{
    void (*draw)(Panel *);
    void (*hit)(Panel *, Mouse *);
};
/*e: struct [[Canvas]] */

/*s: function [[pl_drawcanvas]] */
void pl_drawcanvas(Panel *p){
    Canvas *c;
    c=p->data;
    if(c->draw) c->draw(p);
}
/*e: function [[pl_drawcanvas]] */
/*s: function [[pl_hitcanvas]] */
int pl_hitcanvas(Panel *p, Mouse *m){
    Canvas *c;
    c=p->data;
    if(c->hit) c->hit(p, m);
    return 0;
}
/*e: function [[pl_hitcanvas]] */
/*s: function [[pl_typecanvas]] */
void pl_typecanvas(Panel *p, Rune c){
    USED(p, c);
}
/*e: function [[pl_typecanvas]] */
/*s: function [[pl_getsizecanvas]] */
Point pl_getsizecanvas(Panel *p, Point children){
    USED(p, children);
    return Pt(0,0);
}
/*e: function [[pl_getsizecanvas]] */
/*s: function [[pl_childspacecanvas]] */
void pl_childspacecanvas(Panel *p, Point *ul, Point *size){
    USED(p, ul, size);
}
/*e: function [[pl_childspacecanvas]] */
/*s: function [[plinitcanvas]] */
void plinitcanvas(Panel *v, int flags, void (*draw)(Panel *), void (*hit)(Panel *, Mouse *)){
    Canvas *c;
    v->flags=flags|LEAF;
    v->draw=pl_drawcanvas;
    v->hit=pl_hitcanvas;
    v->type=pl_typecanvas;
    v->getsize=pl_getsizecanvas;
    v->childspace=pl_childspacecanvas;
    v->kind="canvas";
    c=v->data;
    c->draw=draw;
    c->hit=hit;
}
/*e: function [[plinitcanvas]] */
/*s: function [[plcanvas]] */
Panel *plcanvas(Panel *parent, int flags, void (*draw)(Panel *), void (*hit)(Panel *, Mouse *)){
    Panel *p;
    p=pl_newpanel(parent, sizeof(Canvas));
    plinitcanvas(p, flags, draw, hit);
    return p;
}
/*e: function [[plcanvas]] */
/*e: lib_gui/libpanel/canvas.c */
