/*s: lib_gui/libpanel/slider.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"
typedef struct Slider Slider;
/*s: struct [[Slider]] */
struct Slider{
    int dir;			/* HORIZ or VERT */
    int val;			/* setting, in screen coordinates */
    Point minsize;
    void (*hit)(Panel *, int, int, int);	/* call back to user when slider changes */
    int buttons;
};
/*e: struct [[Slider]] */
/*s: function [[pl_drawslider]] */
void pl_drawslider(Panel *p){
    Rectangle r;
    Slider *sp;
    sp=p->data;
    r=pl_box(p->b, p->r, UP);
    switch(sp->dir){
    case HORIZ: pl_sliderupd(p->b, r, sp->dir, 0, sp->val); break;
    case VERT:  pl_sliderupd(p->b, r, sp->dir, r.max.y-sp->val, r.max.y); break;
    }
}
/*e: function [[pl_drawslider]] */
/*s: function [[pl_hitslider]] */
int pl_hitslider(Panel *p, Mouse *m){
    int oldstate, oldval, len;
    Point ul, size;
    Slider *sp;
    sp=p->data;
    ul=p->r.min;
    size=subpt(p->r.max, p->r.min);
    pl_interior(p->state, &ul, &size);
    oldstate=p->state;
    oldval=sp->val;
    SET(len);
    if(m->buttons&OUT)
        p->state=UP;
    else if(m->buttons&7){
        p->state=DOWN;
        sp->buttons=m->buttons;
        if(sp->dir==HORIZ){
            sp->val=m->xy.x-ul.x;
            len=size.x;
        }
        else{
            sp->val=ul.y+size.y-m->xy.y;
            len=size.y;
        }
        if(sp->val<0) sp->val=0;
        else if(sp->val>len) sp->val=len;
    }
    else	/* mouse inside, but no buttons down */
        p->state=UP;
    if(oldval!=sp->val || oldstate!=p->state) pldraw(p, p->b);
    if(oldval!=sp->val && sp->hit) sp->hit(p, sp->buttons, sp->val, len);
    return 0;
}
/*e: function [[pl_hitslider]] */
/*s: function [[pl_typeslider]] */
void pl_typeslider(Panel *p, Rune c){
    USED(p, c);
}
/*e: function [[pl_typeslider]] */
/*s: function [[pl_getsizeslider]] */
Point pl_getsizeslider(Panel *p, Point children){
    USED(children);
    return pl_boxsize(((Slider *)p->data)->minsize, p->state);
}
/*e: function [[pl_getsizeslider]] */
/*s: function [[pl_childspaceslider]] */
void pl_childspaceslider(Panel *g, Point *ul, Point *size){
    USED(g, ul, size);
}
/*e: function [[pl_childspaceslider]] */
/*s: function [[plinitslider]] */
void plinitslider(Panel *v, int flags, Point size, void (*hit)(Panel *, int, int, int)){
    Slider *sp;
    sp=v->data;
    v->r=Rect(0,0,size.x,size.y);
    v->flags=flags|LEAF;
    v->state=UP;
    v->draw=pl_drawslider;
    v->hit=pl_hitslider;
    v->type=pl_typeslider;
    v->getsize=pl_getsizeslider;
    v->childspace=pl_childspaceslider;
    sp->minsize=size;
    sp->dir=size.x>size.y?HORIZ:VERT;
    sp->hit=hit;
    v->kind="slider";
}
/*e: function [[plinitslider]] */
/*s: function [[plslider]] */
Panel *plslider(Panel *parent, int flags, Point size, void (*hit)(Panel *, int, int, int)){
    Panel *p;
    p=pl_newpanel(parent, sizeof(Slider));
    plinitslider(p, flags, size, hit);
    return p;
}
/*e: function [[plslider]] */
/*s: function [[plsetslider]] */
void plsetslider(Panel *p, int value, int range){
    Slider *sp;
    sp=p->data;
    if(value<0) value=0;
    else if(value>range) value=range;
    if(sp->dir==HORIZ) sp->val=value*(p->r.max.x-p->r.min.x)/range;
    else sp->val=value*(p->r.max.y-p->r.min.y)/range;
}
/*e: function [[plsetslider]] */
/*e: lib_gui/libpanel/slider.c */
