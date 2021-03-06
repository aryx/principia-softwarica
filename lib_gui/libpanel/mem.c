/*s: lib_gui/libpanel/mem.c */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

/*s: function [[pl_emalloc]] */
void *pl_emalloc(int n){
    void *v;

    v=mallocz(n, 1);
    if(v==nil){
        fprint(STDERR, "Can't malloc!\n");
        exits("no mem");
    }
    setmalloctag(v, getcallerpc(&n));
    return v;
}
/*e: function [[pl_emalloc]] */
/*s: function [[pl_erealloc]] */
void *pl_erealloc(void *v, int n)
{
    v=realloc(v, n);
    if(v==nil){
        fprint(STDERR, "Can't realloc!\n");
        exits("no mem");
    }
    setrealloctag(v, getcallerpc(&v));
    return v;
}
/*e: function [[pl_erealloc]] */

/*s: function [[pl_unexpected]] */
void pl_unexpected(Panel *g, char *rou){
    fprint(STDERR, "%s called unexpectedly (%s %lux)\n", 
           rou, g->kind, (ulong)g);
    abort();
}
/*e: function [[pl_unexpected]] */
/*s: function [[pl_drawerror]] */
void pl_drawerror(Panel *g){
    pl_unexpected(g, "draw");
}
/*e: function [[pl_drawerror]] */
/*s: function [[pl_hiterror]] */
int pl_hiterror(Panel *g, Mouse *m){
    USED(m);
    pl_unexpected(g, "hit");
    return 0;
}
/*e: function [[pl_hiterror]] */
/*s: function [[pl_typeerror]] */
void pl_typeerror(Panel *g, Rune c){
    USED(c);
    pl_unexpected(g, "type");
}
/*e: function [[pl_typeerror]] */
/*s: function [[pl_getsizeerror]] */
Point pl_getsizeerror(Panel *g, Point childsize){
    pl_unexpected(g, "getsize");
    return childsize;
}
/*e: function [[pl_getsizeerror]] */
/*s: function [[pl_childspaceerror]] */
void pl_childspaceerror(Panel *g, Point *ul, Vector *size){
    USED(ul, size);
    pl_unexpected(g, "childspace");
}
/*e: function [[pl_childspaceerror]] */
/*s: function [[pl_scrollerror]] */
void pl_scrollerror(Panel *g, int dir, buttons button, int num, int den){
    USED(dir, button, num, den);
    pl_unexpected(g, "scroll");
}
/*e: function [[pl_scrollerror]] */
/*s: function [[pl_setscrollbarerror]] */
void pl_setscrollbarerror(Panel *g, int top, int bot, int den){
    USED(top, bot, den);
    pl_unexpected(g, "setscrollbar");
}
/*e: function [[pl_setscrollbarerror]] */

/*s: function [[pl_prinormal]] */
int pl_prinormal(Panel *, Point){
    return PRI_NORMAL;
}
/*e: function [[pl_prinormal]] */

/*s: function [[pl_newpanel]] */
Panel *pl_newpanel(Panel *parent, int ndata){
    Panel *v;

    /*s: [[pl_newpanel()]] sanity check if can create child on [[parent]] */
    if(parent && parent->flags&LEAF){
        fprint(STDERR, "newpanel: can't create child of %s %lux\n", 
                 parent->kind, (ulong)parent);
        exits("bad newpanel");
    }
    /*e: [[pl_newpanel()]] sanity check if can create child on [[parent]] */
    v=pl_emalloc(sizeof(Panel));

    /*s: [[pl_newpanel()]] set tree fields */
    v->next=nil;
    v->child=nil;
    v->echild=nil;

    v->parent=parent;
    //add_list(v, parent->child)
    if(parent){
        if(parent->child==nil)
            parent->child=v;
        else
            parent->echild->next=v;
        parent->echild=v;
    }
    /*e: [[pl_newpanel()]] set tree fields */
    /*s: [[pl_newpanel()]] set widget-specific data fields */
    if(ndata)
        v->data=pl_emalloc(ndata);
    else
        v->data=nil;
    v->free=nil;
    /*e: [[pl_newpanel()]] set widget-specific data fields */
    /*s: [[pl_newpanel()]] set other fields */
    v->r=Rect(0,0,0,0);
    v->b=nil;
    /*x: [[pl_newpanel()]] set other fields */
    v->flags=NOFLAG;
    /*x: [[pl_newpanel()]] set other fields */
    v->ipad=Pt(0,0);
    v->pad=Pt(0,0);
    /*x: [[pl_newpanel()]] set other fields */
    v->lastmouse=nil;
    /*x: [[pl_newpanel()]] set other fields */
    v->pri=pl_prinormal;
    /*x: [[pl_newpanel()]] set other fields */
    v->sizereq=Pt(0,0);
    /*x: [[pl_newpanel()]] set other fields */
    v->size=Pt(0,0);
    /*x: [[pl_newpanel()]] set other fields */
    v->scrollee=nil;
    v->xscroller=nil;
    v->yscroller=nil;
    /*x: [[pl_newpanel()]] set other fields */
    v->scr.pos=Pt(0,0);
    v->scr.size=Pt(0,0);
    /*e: [[pl_newpanel()]] set other fields */
  
    /*s: [[pl_newpanel()]] set default methods */
    v->draw=pl_drawerror;
    v->hit=pl_hiterror;
    v->type=pl_typeerror;
    /*x: [[pl_newpanel()]] set default methods */
    v->getsize=pl_getsizeerror;
    /*x: [[pl_newpanel()]] set default methods */
    v->childspace=pl_childspaceerror;
    /*x: [[pl_newpanel()]] set default methods */
    v->scroll=pl_scrollerror;
    v->setscrollbar=pl_setscrollbarerror;
    /*x: [[pl_newpanel()]] set default methods */
    v->snarf=nil;
    v->paste=nil;
    /*e: [[pl_newpanel()]] set default methods */

    return v;
}
/*e: function [[pl_newpanel]] */
/*s: function [[plfree]] */
void plfree(Panel *p){
    /*s: [[plfree]] locals */
    Panel *cp, *ncp;
    /*e: [[plfree]] locals */

    if(p==nil)
        return;
    /*s: [[plfree()]] if [[plkbfocus]] */
    if(p==plkbfocus)
        plkbfocus=nil;
    /*e: [[plfree()]] if [[plkbfocus]] */
    /*s: [[plfree()]] free the children */
    for(cp=p->child;cp;cp=ncp){
        ncp=cp->next;
        plfree(cp);
    }
    /*e: [[plfree()]] free the children */
    /*s: [[plfree()]] free the widget-specific data */
    if(p->free) 
        p->free(p);
    if(p->data) 
        free(p->data);
    /*e: [[plfree()]] free the widget-specific data */
    free(p);
}
/*e: function [[plfree]] */
/*e: lib_gui/libpanel/mem.c */
