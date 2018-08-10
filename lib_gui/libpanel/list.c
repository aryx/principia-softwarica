/*s: lib_gui/libpanel/list.c */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

typedef struct List List;

/*s: struct [[List]] */
struct List{
    int len;			/* # of items in list */
    Vector minsize;

    int lo;				/* indices of first, last items displayed */
    // option<int> (NONE = -1)
    int sel;			/* index of hilited item */
    /*s: [[List]] other fields */
    buttons buttons;
    /*x: [[List]] other fields */
    Rectangle listr;
    /*e: [[List]] other fields */

    char* (*gen)(Panel *, int);	/* return text given index or 0 if out of range */
    void (*hit)(Panel *, int, int);	/* call user back on hit */

};
/*e: struct [[List]] */

/*s: function [[pl_listsel]] */
void pl_listsel(Panel *p, int sel, bool on){
    List *lp = p->data;
    int hi;
    Rectangle r;

    hi=lp->lo+(lp->listr.max.y-lp->listr.min.y)/font->height;
    if(lp->lo>=0 && lp->lo<=sel && sel<hi && sel<lp->len){
        r=lp->listr;
        r.min.y+=(sel-lp->lo)*font->height;
        r.max.y=r.min.y+font->height;
        if(on)
            pl_highlight(p->b, r);
        else{
            pl_fill(p->b, r);
            pl_drawicon(p->b, r, PLACEW, NOFLAG, lp->gen(p, sel));
        }
    }
}
/*e: function [[pl_listsel]] */
/*s: function [[pl_liststrings]] */
void pl_liststrings(Panel *p, int lo, int hi, Rectangle r){
    List *lp = p->data;
    char *s;
    int i;
    /*s: [[pl_liststrings()]] other locals */
    Panel *sb = p->yscroller;
    /*e: [[pl_liststrings()]] other locals */

    for(i=lo;i!=hi && (s=lp->gen(p, i));i++){
        r.max.y=r.min.y+font->height;
        pl_drawicon(p->b, r, PLACEW, NOFLAG, s);
        r.min.y+=font->height;
    }
    /*s: [[pl_liststrings()]] draw the selection */
    if(lo<=lp->sel && lp->sel<hi) 
        pl_listsel(p, lp->sel, true);
    /*e: [[pl_liststrings()]] draw the selection */
    /*s: [[pl_liststrings()]] set the scrollbar */
    if(sb && sb->setscrollbar)
        sb->setscrollbar(sb, lp->lo,
            lp->lo+(lp->listr.max.y-lp->listr.min.y)/font->height, lp->len);
    /*e: [[pl_liststrings()]] set the scrollbar */
}
/*e: function [[pl_liststrings]] */
/*s: function [[pl_drawlist]] */
void pl_drawlist(Panel *p){
    List *lp = p->data;

    lp->listr=pl_box(p->b, p->r, UP);
    pl_liststrings(p, lp->lo, lp->lo+(lp->listr.max.y-lp->listr.min.y)/font->height,
        lp->listr);
}
/*e: function [[pl_drawlist]] */
/*s: function [[pl_hitlist]] */
int pl_hitlist(Panel *p, Mouse *m){
    int oldsel;
    bool hitme;
    Point ul;
    Vector size;
    List *lp = p->data;

    hitme=false;
    ul=p->r.min;
    size=subpt(p->r.max, p->r.min);
    pl_interior(p->state, &ul, &size);
    oldsel=lp->sel;

    if(m->buttons&OUT){
        p->state=UP;
        if(m->buttons&~OUT) lp->sel=-1;
    }
    else if(p->state==DOWN || m->buttons&7){
        lp->sel=(m->xy.y-ul.y)/font->height+lp->lo;
        if(m->buttons&7){
            lp->buttons=m->buttons;
            p->state=DOWN;
        }
        else{ // release
            hitme=true;
            p->state=UP;
        }
    }

    if(oldsel!=lp->sel){
        pl_listsel(p, oldsel, false);
        pl_listsel(p, lp->sel, true);
    }
    if(hitme && 0<=lp->sel && lp->sel<lp->len && lp->hit)
        // user callback
        lp->hit(p, lp->buttons, lp->sel);
    return false;
}
/*e: function [[pl_hitlist]] */
/*s: function [[pl_scrolllist]] */
void pl_scrolllist(Panel *p, int dir, buttons buttons, int val, int len){
    List *lp = p->data;
    Point ul;
    Vector size;
    int nlist, nline;
    int oldlo, hi;
    Rectangle r;
    int y;

    oldlo=lp->lo;

    ul=p->r.min;
    size=subpt(p->r.max, p->r.min);
    pl_interior(p->state, &ul, &size);

    nlist=size.y/font->height;

    if(dir==VERT) switch(buttons){
    case CLICK_LEFT:    lp->lo-=nlist*val/len; break;
    case CLICK_MIDDLE:  lp->lo=lp->len*val/len; break;
    case CLICK_RIGHT:	lp->lo+=nlist*val/len; break;
    }
    /*s: [[pl_scrolllist()]] sanitize [[lp->lo]] */
    if(lp->lo<0) lp->lo=0;
    if(lp->lo>=lp->len) lp->lo=lp->len-1;
    /*e: [[pl_scrolllist()]] sanitize [[lp->lo]] */
    if(lp->lo==oldlo) 
        return;
    // else

    p->scr.pos.y=lp->lo;
    r=lp->listr;
    nline=(r.max.y-r.min.y)/font->height;
    hi=lp->lo+nline;

    if(hi<=oldlo || lp->lo>=oldlo+nline){
        // =~ pl_drawlist(), complete redraw
        pl_box(p->b, r, PASSIVE); //BUG? forgot r=pl_box(...)
        pl_liststrings(p, lp->lo, hi, r);
    }
    else 
        /*s: [[pl_scrolllist()]] optimized drawing by copying */
        if(lp->lo<oldlo){
            y=r.min.y+(oldlo-lp->lo)*font->height;
            pl_cpy(p->b, Pt(r.min.x, y), 
                Rect(r.min.x, r.min.y, r.max.x, r.min.y+(hi-oldlo)*font->height));
            r.max.y=y;
            pl_box(p->b, r, PASSIVE);
            pl_liststrings(p, lp->lo, oldlo, r);
        }
        else{
            pl_cpy(p->b, r.min, Rect(r.min.x, r.min.y+(lp->lo-oldlo)*font->height,
                r.max.x, r.max.y));
            r.min.y=r.min.y+(oldlo+nline-lp->lo)*font->height;
            pl_box(p->b, r, PASSIVE);
            pl_liststrings(p, oldlo+nline, hi, r);
        }
        /*e: [[pl_scrolllist()]] optimized drawing by copying */
}
/*e: function [[pl_scrolllist]] */
/*s: function [[pl_typelist]] */
void pl_typelist(Panel *g, Rune c){
    USED(g, c);
}
/*e: function [[pl_typelist]] */
/*s: function [[pl_getsizelist]] */
Vector pl_getsizelist(Panel *p, Vector children){
    USED(children);
    return pl_boxsize(((List *)p->data)->minsize, p->state);
}
/*e: function [[pl_getsizelist]] */
/*s: function [[pl_childspacelist]] */
void pl_childspacelist(Panel *g, Point *ul, Vector *size){
    USED(g, ul, size);
}
/*e: function [[pl_childspacelist]] */
/*s: function [[plinitlist]] */
void plinitlist(Panel *v, int flags, char *(*gen)(Panel *, int), int nlist, void (*hit)(Panel *, int, int)){
    List *lp = v->data;
    /*s: [[plinitlist()]] other locals */
    int wid, max;
    char *str;
    /*e: [[plinitlist()]] other locals */

    v->flags=flags|LEAF;
    v->state=UP;

    v->draw=pl_drawlist;
    v->hit=pl_hitlist;
    v->type=pl_typelist;

    v->getsize=pl_getsizelist;
    v->childspace=pl_childspacelist;

    /*s: [[plinitlist()]] set fields in [[lp]] */
    lp->gen=gen;
    lp->hit=hit;
    /*x: [[plinitlist()]] set fields in [[lp]] */
    /*s: [[plinitlist()]] if [[FILLX]] or [[EXPAND]] */
    if(flags&(FILLX|EXPAND)){
        for(lp->len=0;gen(v, lp->len);lp->len++)
           ;
        lp->minsize=Pt(0, nlist*font->height);
    }
    /*e: [[plinitlist()]] if [[FILLX]] or [[EXPAND]] */
    else{
        max=0;
        for(lp->len=0;str=gen(v, lp->len);lp->len++){
            wid=stringwidth(font, str);
            if(wid>max) max=wid;
        }
        lp->minsize=Pt(max, nlist*font->height);
    }
    /*x: [[plinitlist()]] set fields in [[lp]] */
    lp->lo=0;
    lp->sel=-1;
    /*e: [[plinitlist()]] set fields in [[lp]] */
    /*s: [[plinitlist()]] set scrolling fields */
    v->scroll=pl_scrolllist;
    v->scr.pos=Pt(0,0);
    v->scr.size=Pt(0,lp->len);
    /*e: [[plinitlist()]] set scrolling fields */

    v->kind="list";
}
/*e: function [[plinitlist]] */
/*s: function [[pllist]] */
Panel *pllist(Panel *parent, int flags, char *(*gen)(Panel *, int), int nlist, void (*hit)(Panel *, int, int)){
    Panel *v;

    v=pl_newpanel(parent, sizeof(List));
    plinitlist(v, flags, gen, nlist, hit);
    return v;
}
/*e: function [[pllist]] */
/*e: lib_gui/libpanel/list.c */
