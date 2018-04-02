/*s: rc/tree.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */
#include "y.tab.h"

/*s: global [[treenodes]] */
// list<ref_own<Tree>> (next = Tree.next)
tree *treenodes;
/*e: global [[treenodes]] */
/*s: function [[newtree]] */
/*
 * create and clear a new tree node, and add it
 * to the node list.
 */
tree*
newtree(void)
{
    tree *t = new(tree);
    t->str = nil;
    t->child[0] = t->child[1] = t->child[2] = nil;

    // add_list(t, treenodes)
    t->next = treenodes;
    treenodes = t;

    return t;
}
/*e: function [[newtree]] */

/*s: function [[freenodes]] */
void
freenodes(void)
{
    tree *t, *u;
    for(t = treenodes;t;t = u){
        u = t->next;
        if(t->str)
            efree(t->str);
        efree((char *)t);
    }
    treenodes = nil;
}
/*e: function [[freenodes]] */

/*s: function [[tree1]] */
tree*
tree1(int type, tree *c0)
{
    return tree3(type, c0, (tree *)nil, (tree *)nil);
}
/*e: function [[tree1]] */

/*s: function [[tree2]] */
//@Scheck: used by syn.y
tree* tree2(int type, tree *c0, tree *c1)
{
    return tree3(type, c0, c1, (tree *)nil);
}
/*e: function [[tree2]] */

/*s: function [[tree3]] */
tree*
tree3(int type, tree *c0, tree *c1, tree *c2)
{
    tree *t;

    /*s: [[tree3]] if some empty sequence */
    if(type==';'){
        if(c0==nil)
            return c1;
        if(c1==nil)
            return c0;
    }
    /*e: [[tree3]] if some empty sequence */
    // else
    t = newtree();
    t->type = type;
    t->child[0] = c0;
    t->child[1] = c1;
    t->child[2] = c2;
    return t;
}
/*e: function [[tree3]] */

/*s: function [[mung1]] */
//@Scheck: used by syn.y
tree* mung1(tree *t, tree *c0)
{
    t->child[0] = c0;
    return t;
}
/*e: function [[mung1]] */

/*s: function [[mung2]] */
//@Scheck: used by syn.y
tree* mung2(tree *t, tree *c0, tree *c1)
{
    t->child[0] = c0;
    t->child[1] = c1;
    return t;
}
/*e: function [[mung2]] */

/*s: function [[mung3]] */
//@Scheck: used by syn.y
tree* mung3(tree *t, tree *c0, tree *c1, tree *c2)
{
    t->child[0] = c0;
    t->child[1] = c1;
    t->child[2] = c2;
    return t;
}
/*e: function [[mung3]] */

/*s: function [[epimung]] */
//@Scheck: used by syn.y
tree* epimung(tree *comp, tree *epi)
{
    tree *p;
    if(epi==0)
        return comp;
    for(p = epi;p->child[1];p = p->child[1]);
    p->child[1] = comp;
    return epi;
}
/*e: function [[epimung]] */
/*s: function [[simplemung]] */
/*
 * Add a SIMPLE node at the root of t and percolate all the redirections
 * up to the root.
 */
//@Scheck: used by syn.y
tree* simplemung(tree *t)
{
    tree *u;
    struct Io *s;

    t = tree1(SIMPLE, t);

    s = openstr();
    pfmt(s, "%t", t);
    t->str = strdup((char *)s->strp);
    closeio(s);

    /*s: [[simplemung()]] percolate redirections up to the root */
    for(u = t->child[0]; u->type==ARGLIST; u = u->child[0]){
        if(u->child[1]->type==REDIR || u->child[1]->type==DUP){
            u->child[1]->child[1] = t;
            t = u->child[1];
            u->child[1] = nil;
        }
    }
    /*e: [[simplemung()]] percolate redirections up to the root */
    return t;
}
/*e: function [[simplemung]] */

/*e: rc/tree.c */
