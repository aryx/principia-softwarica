/*s: lib_gui/libpanel/pack.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"
/*s: function pl_max */
int pl_max(int a, int b){
    return a>b?a:b;
}
/*e: function pl_max */
/*s: function pl_sizesibs */
Point pl_sizesibs(Panel *p){
    Point s;
    if(p==0) return Pt(0,0);
    s=pl_sizesibs(p->next);
    switch(p->flags&PACK){
    case PACKN:
    case PACKS:
        s.x=pl_max(s.x, p->sizereq.x);
        s.y+=p->sizereq.y;
        break;
    case PACKE:
    case PACKW:
        s.x+=p->sizereq.x;
        s.y=pl_max(s.y, p->sizereq.y);
        break;
    }
    return s;
}
/*e: function pl_sizesibs */
/*s: function pl_sizereq */
/*
 * Compute the requested size of p and its descendants.
 */
void pl_sizereq(Panel *p){
    Panel *cp;
    Point maxsize;
    maxsize=Pt(0,0);
    for(cp=p->child;cp;cp=cp->next){
        pl_sizereq(cp);
        if(cp->sizereq.x>maxsize.x) maxsize.x=cp->sizereq.x;
        if(cp->sizereq.y>maxsize.y) maxsize.y=cp->sizereq.y;
    }
    for(cp=p->child;cp;cp=cp->next){
        if(cp->flags&MAXX) cp->sizereq.x=maxsize.x;
        if(cp->flags&MAXY) cp->sizereq.y=maxsize.y;
    }
    p->childreq=pl_sizesibs(p->child);
    p->sizereq=addpt(addpt(p->getsize(p, p->childreq), p->ipad), p->pad);
    if(p->flags&FIXEDX) p->sizereq.x=p->fixedsize.x;
    if(p->flags&FIXEDY) p->sizereq.y=p->fixedsize.y;
}
/*e: function pl_sizereq */
/*s: function pl_getshare */
Point pl_getshare(Panel *p){
    Point share;
    if(p==0) return Pt(0,0);
    share=pl_getshare(p->next);
    if(p->flags&EXPAND) switch(p->flags&PACK){
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
    return share;
}
/*e: function pl_getshare */
/*s: function pl_setrect */
/*
 * Set the sizes and rectangles of p and its descendants, given their requested sizes.
 */
void pl_setrect(Panel *p, Point ul, Point avail){
    Point space, newul, newspace, slack, share;
    int l;
    Panel *c;
    p->size=subpt(p->sizereq, p->pad);
    ul=addpt(ul, divpt(p->pad, 2));
    avail=subpt(avail, p->pad);
    if(p->size.x>avail.x)
        p->size.x = avail.x;
    if(p->size.y>avail.y)
        p->size.y = avail.y;
    if(p->flags&(FILLX|EXPAND)) p->size.x=avail.x;
    if(p->flags&(FILLY|EXPAND)) p->size.y=avail.y;
    switch(p->flags&PLACE){
    case PLACECEN:	ul.x+=(avail.x-p->size.x)/2; ul.y+=(avail.y-p->size.y)/2; break;
    case PLACES:	ul.x+=(avail.x-p->size.x)/2; ul.y+= avail.y-p->size.y   ; break;
    case PLACEE:	ul.x+= avail.x-p->size.x   ; ul.y+=(avail.y-p->size.y)/2; break;
    case PLACEW:	                             ul.y+=(avail.y-p->size.y)/2; break;
    case PLACEN:	ul.x+=(avail.x-p->size.x)/2;                              break;
    case PLACENE:	ul.x+= avail.x-p->size.x   ;                              break;
    case PLACENW:                                                             break;
    case PLACESE:	ul.x+= avail.x-p->size.x   ; ul.y+= avail.y-p->size.y   ; break;
    case PLACESW:                                ul.y+= avail.y-p->size.y   ; break;
    }
    p->r=Rpt(ul, addpt(ul, p->size));
    space=p->size;
    p->childspace(p, &ul, &space);
    slack=subpt(space, p->childreq);
    share=pl_getshare(p->child);
    for(c=p->child;c;c=c->next){
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
        switch(c->flags&PACK){
        case PACKN:
            newul=Pt(ul.x, ul.y+c->sizereq.y);
            newspace=Pt(space.x, space.y-c->sizereq.y);
            pl_setrect(c, ul, Pt(space.x, c->sizereq.y));
            break;
        case PACKW:
            newul=Pt(ul.x+c->sizereq.x, ul.y);
            newspace=Pt(space.x-c->sizereq.x, space.y);
            pl_setrect(c, ul, Pt(c->sizereq.x, space.y));
            break;
        case PACKS:
            newul=ul;
            newspace=Pt(space.x, space.y-c->sizereq.y);
            pl_setrect(c, Pt(ul.x, ul.y+space.y-c->sizereq.y),
                Pt(space.x, c->sizereq.y));
            break;
        case PACKE:
            newul=ul;
            newspace=Pt(space.x-c->sizereq.x, space.y);
            pl_setrect(c, Pt(ul.x+space.x-c->sizereq.x, ul.y),
                Pt(c->sizereq.x, space.y));
            break;
        }
        ul=newul;
        space=newspace;
    }
}
/*e: function pl_setrect */
/*s: function plpack */
void plpack(Panel *p, Rectangle where){
    pl_sizereq(p);
    pl_setrect(p, where.min, subpt(where.max, where.min));
}
/*e: function plpack */
/*s: function plmove */
/*
 * move an already-packed panel so that p->r=raddp(p->r, d)
 */
void plmove(Panel *p, Point d){
    if(strcmp(p->kind, "edit") == 0)	/* sorry */
        plemove(p, d);
    p->r=rectaddpt(p->r, d);
    for(p=p->child;p;p=p->next) plmove(p, d);
}
/*e: function plmove */
/*e: lib_gui/libpanel/pack.c */
