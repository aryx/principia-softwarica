/*s: lib_gui/libpanel/entry.c */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */
#include <keyboard.h>

typedef struct Entry Entry;

/*s: struct [[Entry]] */
struct Entry{
    // ref_own<string>
    char *entry;
    /*s: [[Entry]] pointer in [[entry]] fields */
    // point to \0 in entry
    char *entp;
    // point to end of entry
    char *eent;
    /*e: [[Entry]] pointer in [[entry]] fields */

    void (*hit)(Panel *, char *);

    Vector minsize;
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
    Entry *ep = p->data;
    char *s;

    r=pl_box(p->b, p->r, p->state);

    s=ep->entry;
    /*s: [[pl_drawentry]] if [[USERFL]] */
    if(p->flags & USERFL){
        char *p;
        s=strdup(s);
        for(p=s; *p; p++)
            *p='*';
    }
    /*e: [[pl_drawentry]] if [[USERFL]] */
    if(stringwidth(font, s) <= r.max.x-r.min.x)
        pl_drawicon(p->b, r, PLACEW, NOFLAG, s);
    else
        pl_drawicon(p->b, r, PLACEE, NOFLAG, s);
    /*s: [[pl_drawentry]] when [[USERFL]] if [[s]] changed */
    if(s != ep->entry)
        free(s);
    /*e: [[pl_drawentry]] when [[USERFL]] if [[s]] changed */
}
/*e: function [[pl_drawentry]] */
/*s: function [[pl_hitentry]] */
bool pl_hitentry(Panel *p, Mouse *m){
    if((m->buttons&7)==CLICK_LEFT){
        plgrabkb(p);

        p->state=DOWN;
        pldraw(p, p->b);

        while(m->buttons&CLICK_LEFT){
            int old = m->buttons;
            // next mouse event
            *m=emouse();
            /*s: [[pl_hitentry()]] handle copy/paste when middle or right click */
            if((old&7)==CLICK_LEFT){
                if((m->buttons&7)==CLICK_LEFT|CLICK_MIDDLE){
                    Entry *ep;

                    plsnarf(p);

                    /* cut */
                    ep=p->data;
                    ep->entp=ep->entry;
                    *ep->entp='\0';
                    pldraw(p, p->b);
                }
                if((m->buttons&7)==CLICK_LEFT|CLICK_RIGHT)
                    plpaste(p);
            }
            /*e: [[pl_hitentry()]] handle copy/paste when middle or right click */
        }

        p->state=UP;
        pldraw(p, p->b);
    }
    return false;
}
/*e: function [[pl_hitentry]] */
/*s: function [[pl_typeentry]] */
void pl_typeentry(Panel *p, Rune c){
    Entry *ep = p->data;
    int n;

    switch(c){
    /*s: [[pl_typeentry()]] switch rune cases */
    case '\n':
    case '\r':
        *ep->entp='\0';
        if(ep->hit) 
            ep->hit(p, ep->entry);
        return;
    /*x: [[pl_typeentry()]] switch rune cases */
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
    /*x: [[pl_typeentry()]] switch rune cases */
    case Kesc:
        plsnarf(p);
        /* no break */
    case Kdel:	/* clear */
    case Kbs:	/* ^H: erase character */
        while(ep->entp!=ep->entry && !pl_rune1st(ep->entp[-1])) 
            *--ep->entp='\0';
        if(ep->entp!=ep->entry) 
            *--ep->entp='\0';
        break;
    /*x: [[pl_typeentry()]] switch rune cases */
    //	case Knack:	/* ^U: erase line */
    //		ep->entp=ep->entry;
    //		*ep->entp='\0';
    //		break;
    //	case Ketb:	/* ^W: erase word */
    //		while(ep->entp!=ep->entry && !pl_idchar(ep->entp[-1]))
    //			--ep->entp;
    //		while(ep->entp!=ep->entry && pl_idchar(ep->entp[-1]))
    //			--ep->entp;
    //		*ep->entp='\0';
    //		break;
    /*e: [[pl_typeentry()]] switch rune cases */
    }
    pldraw(p, p->b);
}
/*e: function [[pl_typeentry]] */
/*s: function [[pl_getsizeentry]] */
Vector pl_getsizeentry(Panel *p, Vector children){
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
    Entry *ep = p->data;

    free(ep->entry);
    ep->entry = ep->eent = ep->entp = nil;
}
/*e: function [[pl_freeentry]] */
/*s: function [[plinitentry]] */
void plinitentry(Panel *v, int flags, int wid, char *str, void (*hit)(Panel *, char *)){
    int elen;
    Entry *ep = v->data;

    v->flags=flags|LEAF;

    v->draw=pl_drawentry;
    v->hit=pl_hitentry;
    v->type=pl_typeentry;

    v->getsize=pl_getsizeentry;
    v->childspace=pl_childspaceentry;

    /*s: [[plinitentry()]] set snarf methods */
    v->snarf=pl_snarfentry;
    v->paste=pl_pasteentry;
    /*e: [[plinitentry()]] set snarf methods */
    /*s: [[plinitentry()]] set extra fields */
    v->state=UP;
    /*e: [[plinitentry()]] set extra fields */
    /*s: [[plinitentry()]] set fields in [[ep]] */
    ep->minsize=Pt(wid, font->height);

    elen=100;
    if(str) 
        elen+=strlen(str);
    ep->entry=pl_erealloc(ep->entry, elen+SLACK);

    ep->eent=ep->entry+elen;
    strecpy(ep->entry, ep->eent, str ? str : "");
    ep->entp=ep->entry+strlen(ep->entry);

    ep->hit=hit;
    /*e: [[plinitentry()]] set fields in [[ep]] */

    v->free=pl_freeentry;

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
    Entry *ep = p->data;

    *ep->entp='\0';
    return ep->entry;
}
/*e: function [[plentryval]] */
/*e: lib_gui/libpanel/entry.c */
