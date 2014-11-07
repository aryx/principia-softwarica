/*s: mk/recipe.c */
#include	"mk.h"

void	addw(Word*, char*);

/*s: function dorecipe */
bool
dorecipe(Node *node)
{
    bool did = false;
    char buf[BIGBLOCK], cwd[256];
    Arc *a, *aa;
    Node *n;
    Rule *r = nil;
    Symtab *s;
    Word head, ahead, lp, ln, *w, *ww, *aw;

    aa = nil;
    /*
     *   pick up the rule
     */
    for(a = node->prereqs; a; a = a->next)
        if(*a->r->recipe) {
            aa = a;
            r = aa->r;
        }
    /*
     *   no recipe? go to buggery!
     */
    if(r == nil){
        if(!(node->flags&VIRTUAL) && !(node->flags&NORECIPE)){
            if(getwd(cwd, sizeof cwd))
                fprint(STDERR, "mk: no recipe to make '%s' in directory %s\n", node->name, cwd);
            else
                fprint(STDERR, "mk: no recipe to make '%s'\n", node->name);
            Exit();
        }
        if(strchr(node->name, '(') && node->time == 0)
            MADESET(node, MADE);
        else
            update(false, node);
        if(tflag){
            if(!(node->flags&VIRTUAL))
                touch(node->name);
            else if(explain)
                Bprint(&bout, "no touch of virtual '%s'\n", node->name);
        }
        return did;
    }

    /*
     *   build the node list
     */
    node->next = 0;
    head.next = 0;
    ww = &head;
    ahead.next = 0;
    aw = &ahead;
    if(r->attr&REGEXP){
        ww->next = newword(node->name);
        aw->next = newword(node->name);
    } else {
        for(w = r->alltargets; w; w = w->next){
            if(r->attr&META)
                subst(aa->stem, w->s, buf, sizeof(buf));
            else
                strecpy(buf, buf + sizeof buf - 1, w->s);
            aw->next = newword(buf);
            aw = aw->next;
            if((s = symlook(buf, S_NODE, 0)) == 0)
                continue;	/* not a node we are interested in */
            n = s->u.ptr;
            if(aflag == false && n->time) {
                for(a = n->prereqs; a; a = a->next)
                    if(a->n && outofdate(n, a, false))
                        break;
                if(a == 0)
                    continue;
            }
            ww->next = newword(buf);
            ww = ww->next;
            if(n == node) continue;
            n->next = node->next;
            node->next = n;
        }
    }
    for(n = node; n; n = n->next)
        if((n->flags&READY) == 0)
            return did;
    /*
        gather the params for the job
    */
    lp.next = ln.next = 0;
    for(n = node; n; n = n->next){
        for(a = n->prereqs; a; a = a->next){
            if(a->n){
                addw(&lp, a->n->name);
                if(outofdate(n, a, false)){
                    addw(&ln, a->n->name);
                    if(explain)
                        fprint(1, "%s(%ld) < %s(%ld)\n",
                            n->name, n->time, a->n->name, a->n->time);
                }
            } else {
                if(explain)
                    fprint(1, "%s has no prerequisites\n",
                            n->name);
            }
        }
        MADESET(n, BEINGMADE);
    }
/*print("lt=%s ln=%s lp=%s\n",wtos(head.next, ' '),wtos(ln.next, ' '),wtos(lp.next, ' '));/**/
    run(newjob(r, node, aa->stem, aa->match, lp.next, ln.next, head.next, ahead.next));
    return true;
}
/*e: function dorecipe */

/*s: function addw */
void
addw(Word *w, char *s)
{
    Word *lw;

    for(lw = w; w = w->next; lw = w){
        if(strcmp(s, w->s) == 0)
            return;
    }
    lw->next = newword(s);
}
/*e: function addw */
/*e: mk/recipe.c */
