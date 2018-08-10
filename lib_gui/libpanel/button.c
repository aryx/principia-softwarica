/*s: lib_gui/libpanel/button.c */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

typedef struct Button Button;

/*s: struct [[Button]] */
struct Button{
    // enum<ButtonType>
    int btype;			/* button type */

    Icon *icon;			/* what to write on the button */

    /*s: [[Button]] other fields */
    // state of buttons when hit
    buttons buttons;
    /*x: [[Button]] other fields */
    bool check;			/* for check/radio buttons */
    /*x: [[Button]] other fields */
    int index;			/* arg to menuhit */
    /*e: [[Button]] other fields */

    void (*pl_buttonhit)(Panel *, buttons);	/* call back user code on button hit */
    void (*hit)(Panel *, buttons, bool);	/* call back user code on check/radio hit */
    /*s: [[Button]] other methods */
    void (*menuhit)(buttons, int);	/* call back user code on menu item hit */
    /*e: [[Button]] other methods */
};
/*e: struct [[Button]] */

/*
 * Button types
 */
/*s: constant [[BUTTON]] */
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
    Button *bp = p->data;

    r=pl_box(p->b, p->r, p->state);
    switch(bp->btype){
    /*s: [[pl_drawbutton()]] switch button type cases */
    case BUTTON:
        break;
    /*x: [[pl_drawbutton()]] switch button type cases */
    case CHECK:
        r=pl_check(p->b, r, bp->check);
        break;
    /*x: [[pl_drawbutton()]] switch button type cases */
    case RADIO:
        r=pl_radio(p->b, r, bp->check);
        break;
    /*e: [[pl_drawbutton()]] switch button type cases */
    }
    pl_drawicon(p->b, r, PLACECEN, p->flags, bp->icon);
}
/*e: function [[pl_drawbutton]] */
/*s: function [[pl_hitbutton]] */
bool pl_hitbutton(Panel *p, Mouse *m){
    Button *bp = p->data;
    int oldstate = p->state;
    bool hitme;
    /*s: [[pl_hitbutton()]] other locals */
    Panel *sib;
    /*e: [[pl_hitbutton()]] other locals */

    if(m->buttons&OUT){ // mouse leaving the widget
        p->state=UP;
        hitme=false;
    }
    else if(m->buttons&7){ // mouse click inside
        p->state=DOWN;
        hitme=false;
        bp->buttons=m->buttons; // remember for the release event
    }
    else{	/* mouse inside, but no buttons down */ // possibly a release
        hitme=p->state==DOWN;
        p->state=UP;
    }

    if(hitme) {
        switch(bp->btype){
        /*s: [[pl_hitbutton()]] switch button type cases */
        case CHECK:
            if(hitme) 
                bp->check=!bp->check;
            break;
        /*x: [[pl_hitbutton()]] switch button type cases */
        case RADIO:
            if(bp->check) 
                bp->check=false;
            else{
                if(p->parent){
                    for(sib=p->parent->child;sib;sib=sib->next){
                        if(sib->hit==pl_hitbutton
                        && ((Button *)sib->data)->btype==RADIO
                        && ((Button *)sib->data)->check){
                            ((Button *)sib->data)->check=false;
                            pldraw(sib, p->b);
                        }
                    }
                }
                bp->check=true;
            }
            break;
        /*e: [[pl_hitbutton()]] switch button type cases */
        }
    }
    if(hitme || oldstate!=p->state) 
        pldraw(p, p->b);
    if(hitme && bp->hit){
        // user callback
        bp->hit(p, bp->buttons, bp->check);
        p->state=UP;
    }
    return false;
}
/*e: function [[pl_hitbutton]] */
/*s: function [[pl_typebutton]] */
void pl_typebutton(Panel *g, Rune c){
    USED(g, c);
}
/*e: function [[pl_typebutton]] */
/*s: function [[pl_getsizebutton]] */
Vector pl_getsizebutton(Panel *p, Vector children){
    Button *bp = p->data;
    Vector s;
    /*s: [[pl_getsizebutton()]] other locals */
    int ckw;
    /*e: [[pl_getsizebutton()]] other locals */

    USED(children);		/* shouldn't have any children */
    s=pl_iconsize(p->flags, bp->icon);
    /*s: [[pl_getsizebutton()]] if not a [[BUTTON]] */
    if(bp->btype!=BUTTON){
        ckw=pl_ckwid();
        if(s.y<ckw){
            s.x+=ckw;
            s.y=ckw;
        }
        else s.x+=s.y;
    }
    /*e: [[pl_getsizebutton()]] if not a [[BUTTON]] */
    return pl_boxsize(s, p->state);
}
/*e: function [[pl_getsizebutton]] */
/*s: function [[pl_childspacebutton]] */
void pl_childspacebutton(Panel *g, Point *ul, Vector *size){
    USED(g, ul, size);
}
/*e: function [[pl_childspacebutton]] */
/*s: function [[pl_initbtype]] */
void pl_initbtype(Panel *v, int flags, Icon *icon, void (*hit)(Panel *, buttons, bool), int btype){
    Button *bp = v->data;

    v->flags=flags|LEAF;
    v->state=UP;

    v->draw=pl_drawbutton;
    v->hit=pl_hitbutton;
    v->type=pl_typebutton;

    v->getsize=pl_getsizebutton;
    v->childspace=pl_childspacebutton;

    bp->btype=btype;
    bp->check=false;
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
void pl_buttonhit(Panel *p, buttons buttons, bool check){
    Button* b = p->data;

    USED(check);
    if(b->pl_buttonhit) 
        b->pl_buttonhit(p, buttons);
}
/*e: function [[pl_buttonhit]] */
/*s: function [[plinitbutton]] */
void plinitbutton(Panel *p, int flags, Icon *icon, void (*hit)(Panel *, buttons)){
    Button* b = p->data;

    b->pl_buttonhit=hit;
    pl_initbtype(p, flags, icon, pl_buttonhit, BUTTON);
}
/*e: function [[plinitbutton]] */
/*s: function [[plinitcheckbutton]] */
void plinitcheckbutton(Panel *p, int flags, Icon *icon, void (*hit)(Panel *, buttons, bool)){
    pl_initbtype(p, flags, icon, hit, CHECK);
}
/*e: function [[plinitcheckbutton]] */
/*s: function [[plinitradiobutton]] */
void plinitradiobutton(Panel *p, int flags, Icon *icon, void (*hit)(Panel *, buttons, bool)){
    pl_initbtype(p, flags, icon, hit, RADIO);
}
/*e: function [[plinitradiobutton]] */
/*s: function [[plbutton]] */
Panel *plbutton(Panel *parent, int flags, Icon *icon, void (*hit)(Panel *, buttons)){
    Panel *p;

    p=pl_newpanel(parent, sizeof(Button));
    plinitbutton(p, flags, icon, hit);
    return p;
}
/*e: function [[plbutton]] */
/*s: function [[plcheckbutton]] */
Panel *plcheckbutton(Panel *parent, int flags, Icon *icon, void (*hit)(Panel *, buttons, bool)){
    Panel *p;

    p=pl_newpanel(parent, sizeof(Button));
    plinitcheckbutton(p, flags, icon, hit);
    return p;
}
/*e: function [[plcheckbutton]] */
/*s: function [[plradiobutton]] */
Panel *plradiobutton(Panel *parent, int flags, Icon *icon, void (*hit)(Panel *, buttons, bool)){
    Panel *p;

    p=pl_newpanel(parent, sizeof(Button));
    plinitradiobutton(p, flags, icon, hit);
    return p;
}
/*e: function [[plradiobutton]] */

/*s: function [[pl_hitmenu]] */
void pl_hitmenu(Panel *p, buttons buttons){
    Button* b = p->data;
    void (*hit)(int, int) = b->menuhit;

    if(hit) 
        hit(buttons, b->index);
}
/*e: function [[pl_hitmenu]] */
/*s: function [[plinitmenu]] */
void plinitmenu(Panel *v, int flags, Icon **item, int cflags, void (*hit)(buttons, int)){
    Panel *p;
    Button* b;
    int i;

    v->flags=flags;

    /*s: [[plinitmenu()]] free child widget if any */
    if(v->child){
        plfree(v->child);
        v->child=nil;
    }
    /*e: [[plinitmenu()]] free child widget if any */

    for(i=0;item[i];i++){
        p=plbutton(v, cflags, item[i], pl_hitmenu);
        b = p->data;

        b->menuhit=hit;
        b->index=i;
    }
    v->kind="menu";
}
/*e: function [[plinitmenu]] */
/*s: function [[plmenu]] */
Panel *plmenu(Panel *parent, int flags, Icon **item, int cflags, void (*hit)(buttons, int)){
    Panel *v;

    v=plgroup(parent, flags);
    plinitmenu(v, flags, item, cflags, hit);
    return v;
}
/*e: function [[plmenu]] */
/*s: function [[plsetbutton]] */
void plsetbutton(Panel *p, int val){
    Button* b = p->data;

    b->check=val;
}
/*e: function [[plsetbutton]] */
/*e: lib_gui/libpanel/button.c */
