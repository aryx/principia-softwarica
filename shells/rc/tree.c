/*s: rc/tree.c */
#include "rc.h"
#include "exec.h"
#include "io.h"
#include "fns.h"
/*s: global treenodes */
tree *treenodes;
/*e: global treenodes */
/*s: function newtree */
/*
 * create and clear a new tree node, and add it
 * to the node list.
 */

tree*
newtree(void)
{
    tree *t = new(tree);
    t->iskw = 0;
    t->str = 0;
    t->child[0] = t->child[1] = t->child[2] = 0;
    t->next = treenodes;
    treenodes = t;
    return t;
}
/*e: function newtree */

/*s: function freenodes */
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
    treenodes = 0;
}
/*e: function freenodes */

/*s: function tree1 */
tree*
tree1(int type, tree *c0)
{
    return tree3(type, c0, (tree *)0, (tree *)0);
}
/*e: function tree1 */

/*s: function tree2 */
tree*
tree2(int type, tree *c0, tree *c1)
{
    return tree3(type, c0, c1, (tree *)0);
}
/*e: function tree2 */

/*s: function tree3 */
tree*
tree3(int type, tree *c0, tree *c1, tree *c2)
{
    tree *t;
    if(type==';'){
        if(c0==0)
            return c1;
        if(c1==0)
            return c0;
    }
    t = newtree();
    t->type = type;
    t->child[0] = c0;
    t->child[1] = c1;
    t->child[2] = c2;
    return t;
}
/*e: function tree3 */

/*s: function mung1 */
tree*
mung1(tree *t, tree *c0)
{
    t->child[0] = c0;
    return t;
}
/*e: function mung1 */

/*s: function mung2 */
tree*
mung2(tree *t, tree *c0, tree *c1)
{
    t->child[0] = c0;
    t->child[1] = c1;
    return t;
}
/*e: function mung2 */

/*s: function mung3 */
tree*
mung3(tree *t, tree *c0, tree *c1, tree *c2)
{
    t->child[0] = c0;
    t->child[1] = c1;
    t->child[2] = c2;
    return t;
}
/*e: function mung3 */

/*s: function epimung */
tree*
epimung(tree *comp, tree *epi)
{
    tree *p;
    if(epi==0)
        return comp;
    for(p = epi;p->child[1];p = p->child[1]);
    p->child[1] = comp;
    return epi;
}
/*e: function epimung */
/*s: function simplemung */
/*
 * Add a SIMPLE node at the root of t and percolate all the redirections
 * up to the root.
 */

tree*
simplemung(tree *t)
{
    tree *u;
    struct io *s;

    t = tree1(SIMPLE, t);
    s = openstr();
    pfmt(s, "%t", t);
    t->str = strdup((char *)s->strp);
    closeio(s);
    for(u = t->child[0];u->type==ARGLIST;u = u->child[0]){
        if(u->child[1]->type==DUP
        || u->child[1]->type==REDIR){
            u->child[1]->child[1] = t;
            t = u->child[1];
            u->child[1] = 0;
        }
    }
    return t;
}
/*e: function simplemung */

/*s: function token */
tree*
token(char *str, int type)
{
    tree *t = newtree();

    t->type = type;
    t->str = strdup(str);
    return t;
}
/*e: function token */

/*s: function freetree */
void
freetree(tree *p)
{
    if(p==0)
        return;	
    freetree(p->child[0]);
    freetree(p->child[1]);
    freetree(p->child[2]);
    if(p->str)
        efree(p->str);
    efree((char *)p);
}
/*e: function freetree */
/*e: rc/tree.c */
