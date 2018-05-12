/*s: lib_gui/libpanel/entry.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"
#include <keyboard.h>

typedef struct Entry Entry;
/*s: struct [[Entry]] */
struct Entry{
    char *entry;
    char *entp;
    char *eent;
    void (*hit)(Panel *, char *);
    Point minsize;
};
/*e: struct [[Entry]] */
/*s: constant [[SLACK]] */
#define	SLACK	7	/* enough for one extra rune and ◀ and a nul */
/*e: constant [[SLACK]] */
/*s: function [[pl_snarfentry]] */
char *pl_snarfentry(Panel *p){
    Entry *ep;
    int n;

    if(p->flags&USERFL)	/* no snarfing from password entry */
        return nil;
    ep=p->data;
    n=ep->entp-ep->entry;
    if(n<=0) return nil;
    return smprint("%.*s", n, ep->entry);
}
/*e: function [[pl_snarfentry]] */
/*s: function [[pl_pasteentry]] */
void pl_pasteentry(Panel *p, char *s){
    Entry *ep;
    char *e;
    int n, m;

    ep=p->data;
    n=ep->entp-ep->entry;
    m=strlen(s);
    e=pl_erealloc(ep->entry,n+m+SLACK);
    ep->entry=e;
    e+=n;
    strncpy(e, s, m);
    e+=m;
    *e='\0';
    ep->entp=ep->eent=e;
    pldraw(p, p->b);
}
/*e: function [[pl_pasteentry]] */
/*s: function [[pl_drawentry]] */
void pl_drawentry(Panel *p){
    Rectangle r;
    Entry *ep;
    char *s;

    ep=p->data;
    r=pl_box(p->b, p->r, p->state);
    s=ep->entry;
    if(p->flags & USERFL){
        char *p;
        s=strdup(s);
        for(p=s; *p; p++)
            *p='*';
    }
    if(stringwidth(font, s)<=r.max.x-r.min.x)
        pl_drawicon(p->b, r, PLACEW, 0, s);
    else
        pl_drawicon(p->b, r, PLACEE, 0, s);
    if(s != ep->entry)
        free(s);
}
/*e: function [[pl_drawentry]] */
/*s: function [[pl_hitentry]] */
int pl_hitentry(Panel *p, Mouse *m){
    if((m->buttons&7)==1){
        plgrabkb(p);

        p->state=DOWN;
        pldraw(p, p->b);
        while(m->buttons&1){
            int old;
            old=m->buttons;
            *m=emouse();
            if((old&7)==1){
                if((m->buttons&7)==3){
                    Entry *ep;

                    plsnarf(p);

                    /* cut */
                    ep=p->data;
                    ep->entp=ep->entry;
                    *ep->entp='\0';
                    pldraw(p, p->b);
                }
                if((m->buttons&7)==5)
                    plpaste(p);
            }
        }
        p->state=UP;
        pldraw(p, p->b);
    }
    return 0;
}
/*e: function [[pl_hitentry]] */
/*s: function [[pl_typeentry]] */
void pl_typeentry(Panel *p, Rune c){
    int n;
    Entry *ep;
    ep=p->data;
    switch(c){
    case '\n':
    case '\r':
        *ep->entp='\0';
        if(ep->hit) ep->hit(p, ep->entry);
        return;
    case Kesc:
        plsnarf(p);
        /* no break */
    case Kdel:	/* clear */
//	case Knack:	/* ^U: erase line */
//		ep->entp=ep->entry;
//		*ep->entp='\0';
//		break;
    case Kbs:	/* ^H: erase character */
        while(ep->entp!=ep->entry && !pl_rune1st(ep->entp[-1])) *--ep->entp='\0';
        if(ep->entp!=ep->entry) *--ep->entp='\0';
        break;
//	case Ketb:	/* ^W: erase word */
//		while(ep->entp!=ep->entry && !pl_idchar(ep->entp[-1]))
//			--ep->entp;
//		while(ep->entp!=ep->entry && pl_idchar(ep->entp[-1]))
//			--ep->entp;
//		*ep->entp='\0';
//		break;
    default:
        if(c < 0x20 || (c & 0xFF00) == KF || (c & 0xFF00) == Spec)
            break;
        ep->entp+=runetochar(ep->entp, &c);
        if(ep->entp>ep->eent){
            n=ep->entp-ep->entry;
            ep->entry=pl_erealloc(ep->entry, n+100+SLACK);
            ep->entp=ep->entry+n;
            ep->eent=ep->entp+100;
        }
        *ep->entp='\0';
        break;
    }
    pldraw(p, p->b);
}
/*e: function [[pl_typeentry]] */
/*s: function [[pl_getsizeentry]] */
Point pl_getsizeentry(Panel *p, Point children){
    USED(children);
    return pl_boxsize(((Entry *)p->data)->minsize, p->state);
}
/*e: function [[pl_getsizeentry]] */
/*s: function [[pl_childspaceentry]] */
void pl_childspaceentry(Panel *p, Point *ul, Point *size){
    USED(p, ul, size);
}
/*e: function [[pl_childspaceentry]] */
/*s: function [[pl_freeentry]] */
void pl_freeentry(Panel *p){
    Entry *ep;
    ep = p->data;
    free(ep->entry);
    ep->entry = ep->eent = ep->entp = 0;
}
/*e: function [[pl_freeentry]] */
/*s: function [[plinitentry]] */
void plinitentry(Panel *v, int flags, int wid, char *str, void (*hit)(Panel *, char *)){
    int elen;
    Entry *ep;
    ep=v->data;
    v->flags=flags|LEAF;
    v->state=UP;
    v->draw=pl_drawentry;
    v->hit=pl_hitentry;
    v->type=pl_typeentry;
    v->getsize=pl_getsizeentry;
    v->childspace=pl_childspaceentry;
    ep->minsize=Pt(wid, font->height);
    v->free=pl_freeentry;
    v->snarf=pl_snarfentry;
    v->paste=pl_pasteentry;
    elen=100;
    if(str) elen+=strlen(str);
    ep->entry=pl_erealloc(ep->entry, elen+SLACK);
    ep->eent=ep->entry+elen;
    strecpy(ep->entry, ep->eent, str ? str : "");
    ep->entp=ep->entry+strlen(ep->entry);
    ep->hit=hit;
    v->kind="entry";
}
/*e: function [[plinitentry]] */
/*s: function [[plentry]] */
Panel *plentry(Panel *parent, int flags, int wid, char *str, void (*hit)(Panel *, char *)){
    Panel *v;
    v=pl_newpanel(parent, sizeof(Entry));
    plinitentry(v, flags, wid, str, hit);
    return v;
}
/*e: function [[plentry]] */
/*s: function [[plentryval]] */
char *plentryval(Panel *p){
    Entry *ep;
    ep=p->data;
    *ep->entp='\0';
    return ep->entry;
}
/*e: function [[plentryval]] */
/*e: lib_gui/libpanel/entry.c */
