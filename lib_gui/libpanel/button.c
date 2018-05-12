/*s: lib_gui/libpanel/button.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"
typedef struct Button Button;
/*s: struct [[Button]] */
struct Button{
    int btype;			/* button type */
    Icon *icon;			/* what to write on the button */
    int check;			/* for check/radio buttons */
    void (*hit)(Panel *, int, int);	/* call back user code on check/radio hit */
    void (*menuhit)(int, int);	/* call back user code on menu item hit */
    void (*pl_buttonhit)(Panel *, int);	/* call back user code on button hit */
    int index;			/* arg to menuhit */
    int buttons;
};
/*e: struct [[Button]] */
/*s: constant [[BUTTON]] */
/*
 * Button types
 */
#define	BUTTON	1
/*e: constant [[BUTTON]] */
/*s: constant [[CHECK]] */
#define	CHECK	2
/*e: constant [[CHECK]] */
/*s: constant [[RADIO]] */
#define	RADIO	3
/*e: constant [[RADIO]] */
/*s: function [[pl_drawbutton]] */
void pl_drawbutton(Panel *p){
    Rectangle r;
    Button *bp;
    bp=p->data;
    r=pl_box(p->b, p->r, p->state);
    switch(bp->btype){
    case CHECK:
        r=pl_check(p->b, r, bp->check);
        break;
    case RADIO:
        r=pl_radio(p->b, r, bp->check);
        break;
    }
    pl_drawicon(p->b, r, PLACECEN, p->flags, bp->icon);
}
/*e: function [[pl_drawbutton]] */
/*s: function [[pl_hitbutton]] */
int pl_hitbutton(Panel *p, Mouse *m){
    int oldstate, hitme;
    Panel *sib;
    Button *bp;
    bp=p->data;
    oldstate=p->state;
    if(m->buttons&OUT){
        hitme=0;
        p->state=UP;
    }
    else if(m->buttons&7){
        hitme=0;
        p->state=DOWN;
        bp->buttons=m->buttons;
    }
    else{	/* mouse inside, but no buttons down */
        hitme=p->state==DOWN;
        p->state=UP;
    }
    if(hitme) switch(bp->btype){
    case CHECK:
        if(hitme) bp->check=!bp->check;
        break;
    case RADIO:
        if(bp->check) bp->check=0;
        else{
            if(p->parent){
                for(sib=p->parent->child;sib;sib=sib->next){
                    if(sib->hit==pl_hitbutton
                    && ((Button *)sib->data)->btype==RADIO
                    && ((Button *)sib->data)->check){
                        ((Button *)sib->data)->check=0;
                        pldraw(sib, p->b);
                    }
                }
            }
            bp->check=1;
        }
        break;
    }
    if(hitme || oldstate!=p->state) pldraw(p, p->b);
    if(hitme && bp->hit){
        bp->hit(p, bp->buttons, bp->check);
        p->state=UP;
    }
    return 0;
}
/*e: function [[pl_hitbutton]] */
/*s: function [[pl_typebutton]] */
void pl_typebutton(Panel *g, Rune c){
    USED(g, c);
}
/*e: function [[pl_typebutton]] */
/*s: function [[pl_getsizebutton]] */
Point pl_getsizebutton(Panel *p, Point children){
    Point s;
    int ckw;
    Button *bp;
    USED(children);		/* shouldn't have any children */
    bp=p->data;
    s=pl_iconsize(p->flags, bp->icon);
    if(bp->btype!=BUTTON){
        ckw=pl_ckwid();
        if(s.y<ckw){
            s.x+=ckw;
            s.y=ckw;
        }
        else s.x+=s.y;
    }
    return pl_boxsize(s, p->state);
}
/*e: function [[pl_getsizebutton]] */
/*s: function [[pl_childspacebutton]] */
void pl_childspacebutton(Panel *g, Point *ul, Point *size){
    USED(g, ul, size);
}
/*e: function [[pl_childspacebutton]] */
/*s: function [[pl_initbtype]] */
void pl_initbtype(Panel *v, int flags, Icon *icon, void (*hit)(Panel *, int, int), int btype){
    Button *bp;
    bp=v->data;
    v->flags=flags|LEAF;
    v->state=UP;
    v->draw=pl_drawbutton;
    v->hit=pl_hitbutton;
    v->type=pl_typebutton;
    v->getsize=pl_getsizebutton;
    v->childspace=pl_childspacebutton;
    bp->btype=btype;
    bp->check=0;
    bp->hit=hit;
    bp->icon=icon;
    switch(btype){
    case BUTTON: v->kind="button"; break;
    case CHECK:  v->kind="checkbutton"; break;
    case RADIO:  v->kind="radiobutton"; break;
    }
}
/*e: function [[pl_initbtype]] */
/*s: function [[pl_buttonhit]] */
void pl_buttonhit(Panel *p, int buttons, int check){
    USED(check);
    if(((Button *)p->data)->pl_buttonhit) ((Button *)p->data)->pl_buttonhit(p, buttons);
}
/*e: function [[pl_buttonhit]] */
/*s: function [[plinitbutton]] */
void plinitbutton(Panel *p, int flags, Icon *icon, void (*hit)(Panel *, int)){
    ((Button *)p->data)->pl_buttonhit=hit;
    pl_initbtype(p, flags, icon, pl_buttonhit, BUTTON);
}
/*e: function [[plinitbutton]] */
/*s: function [[plinitcheckbutton]] */
void plinitcheckbutton(Panel *p, int flags, Icon *icon, void (*hit)(Panel *, int, int)){
    pl_initbtype(p, flags, icon, hit, CHECK);
}
/*e: function [[plinitcheckbutton]] */
/*s: function [[plinitradiobutton]] */
void plinitradiobutton(Panel *p, int flags, Icon *icon, void (*hit)(Panel *, int, int)){
    pl_initbtype(p, flags, icon, hit, RADIO);
}
/*e: function [[plinitradiobutton]] */
/*s: function [[plbutton]] */
Panel *plbutton(Panel *parent, int flags, Icon *icon, void (*hit)(Panel *, int)){
    Panel *p;
    p=pl_newpanel(parent, sizeof(Button));
    plinitbutton(p, flags, icon, hit);
    return p;
}
/*e: function [[plbutton]] */
/*s: function [[plcheckbutton]] */
Panel *plcheckbutton(Panel *parent, int flags, Icon *icon, void (*hit)(Panel *, int, int)){
    Panel *p;
    p=pl_newpanel(parent, sizeof(Button));
    plinitcheckbutton(p, flags, icon, hit);
    return p;
}
/*e: function [[plcheckbutton]] */
/*s: function [[plradiobutton]] */
Panel *plradiobutton(Panel *parent, int flags, Icon *icon, void (*hit)(Panel *, int, int)){
    Panel *p;
    p=pl_newpanel(parent, sizeof(Button));
    plinitradiobutton(p, flags, icon, hit);
    return p;
}
/*e: function [[plradiobutton]] */
/*s: function [[pl_hitmenu]] */
void pl_hitmenu(Panel *p, int buttons){
    void (*hit)(int, int);
    hit=((Button *)p->data)->menuhit;
    if(hit) hit(buttons, ((Button *)p->data)->index);
}
/*e: function [[pl_hitmenu]] */
/*s: function [[plinitmenu]] */
void plinitmenu(Panel *v, int flags, Icon **item, int cflags, void (*hit)(int, int)){
    Panel *b;
    int i;
    v->flags=flags;
    v->kind="menu";
    if(v->child){
        plfree(v->child);
        v->child=0;
    }
    for(i=0;item[i];i++){
        b=plbutton(v, cflags, item[i], pl_hitmenu);
        ((Button *)b->data)->menuhit=hit;
        ((Button *)b->data)->index=i;
    }
}
/*e: function [[plinitmenu]] */
/*s: function [[plmenu]] */
Panel *plmenu(Panel *parent, int flags, Icon **item, int cflags, void (*hit)(int, int)){
    Panel *v;
    v=plgroup(parent, flags);
    plinitmenu(v, flags, item, cflags, hit);
    return v;
}
/*e: function [[plmenu]] */
/*s: function [[plsetbutton]] */
void plsetbutton(Panel *p, int val){
    ((Button *)p->data)->check=val;
}
/*e: function [[plsetbutton]] */
/*e: lib_gui/libpanel/button.c */
