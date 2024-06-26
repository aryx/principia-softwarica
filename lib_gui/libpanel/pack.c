/*s: lib_gui/libpanel/pack.c */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

/*s: function [[pl_max]] */
int pl_max(int a, int b){
    return a>b ? a : b;
}
/*e: function [[pl_max]] */
/*s: function [[pl_sizesibs]] */
Vector pl_sizesibs(Panel *p){
    Vector s;

    /*s: [[pl_sizesibs()]] if no panel */
    if(p==nil) 
        return Pt(0,0);
    /*e: [[pl_sizesibs()]] if no panel */
    // recurse
    s=pl_sizesibs(p->next);

    switch(p->flags&PACK){
    /*s: [[pl_sizesibs()]] switch packing flags cases */
    case PACKN:
    case PACKS:
        s.x=pl_max(s.x, p->sizereq.x);
        s.y+=p->sizereq.y;
        break;
    /*x: [[pl_sizesibs()]] switch packing flags cases */
    case PACKE:
    case PACKW:
        s.x+=p->sizereq.x;
        s.y=pl_max(s.y, p->sizereq.y);
        break;
    /*e: [[pl_sizesibs()]] switch packing flags cases */
    }
    return s;
}
/*e: function [[pl_sizesibs]] */
/*s: function [[pl_sizereq]] */
/*
 * Compute the requested size of p and its descendants.
 */
void pl_sizereq(Panel *p){
    Panel *cp;
    /*s: [[pl_sizeref()]] other locals */
    Vector maxsize = Pt(0,0);
    /*e: [[pl_sizeref()]] other locals */

    for(cp=p->child;cp;cp=cp->next){
        // recurse
        pl_sizereq(cp);
        /*s: [[pl_sizeref()]] when looping over children, adjust [[maxsize]] */
        if(cp->sizereq.x>maxsize.x) maxsize.x=cp->sizereq.x;
        if(cp->sizereq.y>maxsize.y) maxsize.y=cp->sizereq.y;
        /*e: [[pl_sizeref()]] when looping over children, adjust [[maxsize]] */
    }
    /*s: [[pl_sizereq()]] if [[MAXX]] or [[MAXY]] */
    for(cp=p->child;cp;cp=cp->next){
        if(cp->flags&MAXX) cp->sizereq.x=maxsize.x;
        if(cp->flags&MAXY) cp->sizereq.y=maxsize.y;
    }
    /*e: [[pl_sizereq()]] if [[MAXX]] or [[MAXY]] */
    p->childreq=pl_sizesibs(p->child);
    // widget-specific method
    p->sizereq=p->getsize(p, p->childreq);
    /*s: [[pl_sizereq()]] adjust size with padding */
    p->sizereq=addpt(addpt(p->sizereq, p->ipad), p->pad);
    /*e: [[pl_sizereq()]] adjust size with padding */
    /*s: [[pl_sizereq()]] if [[FIXEDX]] or [[FIXEDY]] */
    if(p->flags&FIXEDX) p->sizereq.x=p->fixedsize.x;
    if(p->flags&FIXEDY) p->sizereq.y=p->fixedsize.y;
    /*e: [[pl_sizereq()]] if [[FIXEDX]] or [[FIXEDY]] */
}
/*e: function [[pl_sizereq]] */
/*s: function [[pl_getshare]] */
Point pl_getshare(Panel *p){
    Point share;

    if(p==nil) 
        return Pt(0,0);

    // recurse
    share=pl_getshare(p->next);

    if(p->flags&EXPAND) {
        switch(p->flags&PACK){
        case PACKN:
        case PACKS:
            if(share.x==0) share.x=1;
            share.y++;
            break;
        case PACKE:
        case PACKW:
            share.x++;
            if(share.y==0) share.y=1;
            break;
        }
    }
    return share;
}
/*e: function [[pl_getshare]] */
/*s: function [[pl_setrect]] */
/*
 * Set the sizes and rectangles of p and its descendants, given their requested sizes.
 */
void pl_setrect(Panel *p, Point ul, Vector avail){
    /*s: [[pl_setrect()]] locals */
    Panel *c;
    Vector space;
    Point newul;
    Vector newspace;
    /*x: [[pl_setrect()]] locals */
    Vector slack, share;
    int l;
    /*e: [[pl_setrect()]] locals */

    // setting p->size
    p->size=p->sizereq;
    /*s: [[pl_setrect()]] reduce size for external padding */
    p->size=subpt(p->sizereq, p->pad);
    ul=addpt(ul, divpt(p->pad, 2));
    avail=subpt(avail, p->pad);
    /*e: [[pl_setrect()]] reduce size for external padding */
    /*s: [[pl_setrect()]] reduce size if bigger than available */
    if(p->size.x>avail.x)
        p->size.x = avail.x;
    if(p->size.y>avail.y)
        p->size.y = avail.y;
    /*e: [[pl_setrect()]] reduce size if bigger than available */
    /*s: [[pl_setrect()]] expand size if fill or expand flags */
    if(p->flags&(FILLX|EXPAND)) 
        p->size.x=avail.x;
    if(p->flags&(FILLY|EXPAND)) 
        p->size.y=avail.y;
    /*e: [[pl_setrect()]] expand size if fill or expand flags */

    // setting ul
    /*s: [[pl_setrect()]] adjust origin with placement */
    switch(p->flags&PLACE){
    case PLACECEN:	ul.x+=(avail.x-p->size.x)/2; ul.y+=(avail.y-p->size.y)/2; break;
    case PLACES:	ul.x+=(avail.x-p->size.x)/2; ul.y+= avail.y-p->size.y   ; break;
    case PLACEE:	ul.x+= avail.x-p->size.x   ; ul.y+=(avail.y-p->size.y)/2; break;
    case PLACEW:	                             ul.y+=(avail.y-p->size.y)/2; break;
    case PLACEN:	ul.x+=(avail.x-p->size.x)/2;                              break;
    case PLACENE:	ul.x+= avail.x-p->size.x   ;                              break;
    case PLACENW:   /** Nothing **/                                           break;
    case PLACESE:	ul.x+= avail.x-p->size.x   ; ul.y+= avail.y-p->size.y   ; break;
    case PLACESW:                                ul.y+= avail.y-p->size.y   ; break;
    }
    /*e: [[pl_setrect()]] adjust origin with placement */

    // setting p->r
    p->r=Rpt(ul, addpt(ul, p->size));

    // recurse
    /*s: [[pl_setrect()]] setting the rectangle of the children */
    space=p->size;
    // widget-specific adjustment method
    p->childspace(p, &ul, &space);

    /*s: [[pl_setrect()]] before looping over children, set locals */
    slack=subpt(space, p->childreq);
    share=pl_getshare(p->child);
    /*e: [[pl_setrect()]] before looping over children, set locals */
    for(c=p->child;c;c=c->next){
        /*s: [[pl_setrect()]] when looping over children, if [[EXPAND]] flag */
        if(c->flags&EXPAND){
            switch(c->flags&PACK){
            case PACKN:
            case PACKS:
                c->sizereq.x+=slack.x;
                l=slack.y/share.y;
                c->sizereq.y+=l;
                slack.y-=l;
                --share.y;
                break;
            case PACKE:
            case PACKW:
                l=slack.x/share.x;
                c->sizereq.x+=l;
                slack.x-=l;
                --share.x;
                c->sizereq.y+=slack.y;
                break;
            }
        }
        /*e: [[pl_setrect()]] when looping over children, if [[EXPAND]] flag */
        switch(c->flags&PACK){
        /*s: [[pl_setrect()]] when looping over children, switch packing cases */
        case PACKN:
            newul=Pt(ul.x, ul.y+c->sizereq.y);
            newspace=Pt(space.x, space.y-c->sizereq.y);
            pl_setrect(c, ul, Pt(space.x, c->sizereq.y));
            break;
        case PACKS:
            newul=ul;
            newspace=Pt(space.x, space.y-c->sizereq.y);
            pl_setrect(c, Pt(ul.x, ul.y+space.y-c->sizereq.y),
                Pt(space.x, c->sizereq.y));
            break;
        /*x: [[pl_setrect()]] when looping over children, switch packing cases */
        case PACKW:
            newul=Pt(ul.x+c->sizereq.x, ul.y);
            newspace=Pt(space.x-c->sizereq.x, space.y);
            pl_setrect(c, ul, Pt(c->sizereq.x, space.y));
            break;
        case PACKE:
            newul=ul;
            newspace=Pt(space.x-c->sizereq.x, space.y);
            pl_setrect(c, Pt(ul.x+space.x-c->sizereq.x, ul.y),
                Pt(c->sizereq.x, space.y));
            break;
        /*e: [[pl_setrect()]] when looping over children, switch packing cases */
        }
        ul=newul;
        space=newspace;
    }
    /*e: [[pl_setrect()]] setting the rectangle of the children */
}
/*e: function [[pl_setrect]] */
/*s: function [[plpack]] */
void plpack(Panel *p, Rectangle where){
    pl_sizereq(p);
    pl_setrect(p, where.min, subpt(where.max, where.min));
}
/*e: function [[plpack]] */
/*s: function [[plmove]] */
/*
 * move an already-packed panel so that p->r=raddp(p->r, d)
 */
void plmove(Panel *p, Point d){
    /*s: [[plmove()]] if edit widget special case */
    if(strcmp(p->kind, "edit") == 0)	/* sorry */
        plemove(p, d);
    /*e: [[plmove()]] if edit widget special case */
    p->r=rectaddpt(p->r, d);
    for(p=p->child;p;p=p->next) 
        plmove(p, d);
}
/*e: function [[plmove]] */
/*e: lib_gui/libpanel/pack.c */
