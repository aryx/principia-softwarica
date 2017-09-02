/*s: windows/libpanel/group.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"
/*s: function pl_drawgroup */
void pl_drawgroup(Panel *p){
    USED(p);
}
/*e: function pl_drawgroup */
/*s: function pl_hitgroup */
int pl_hitgroup(Panel *p, Mouse *m){
    USED(p, m);
    return 0;
}
/*e: function pl_hitgroup */
/*s: function pl_typegroup */
void pl_typegroup(Panel *p, Rune c){
    USED(p, c);
}
/*e: function pl_typegroup */
/*s: function pl_getsizegroup */
Point pl_getsizegroup(Panel *p, Point children){
    USED(p);
    return children;
}
/*e: function pl_getsizegroup */
/*s: function pl_childspacegroup */
void pl_childspacegroup(Panel *p, Point *ul, Point *size){
    USED(p, ul, size);
}
/*e: function pl_childspacegroup */
/*s: function plinitgroup */
void plinitgroup(Panel *v, int flags){
    v->flags=flags;
    v->draw=pl_drawgroup;
    v->hit=pl_hitgroup;
    v->type=pl_typegroup;
    v->getsize=pl_getsizegroup;
    v->childspace=pl_childspacegroup;
    v->kind="group";
}
/*e: function plinitgroup */
/*s: function plgroup */
Panel *plgroup(Panel *parent, int flags){
    Panel *p;
    p=pl_newpanel(parent, 0);
    plinitgroup(p, flags);
    return p;
}
/*e: function plgroup */
/*e: windows/libpanel/group.c */
