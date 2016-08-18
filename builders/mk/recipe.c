/*s: mk/recipe.c */
#include	"mk.h"

/*s: constructor newjob */
Job*
newjob(Rule *r, Node *nlist, char *stem, char **match, 
       Word *pre, Word *npre, 
       Word *targets, Word *atar)
{
    Job *j;

    j = (Job *)Malloc(sizeof(Job));
    j->r = r;
    j->n = nlist;
    j->p = pre;
    j->t = targets;

    j->stem = stem;
    j->match = match;

    j->np = npre;
    j->at = atar;

    j->next = nil;
    return j;
}
/*e: constructor newjob */

/*s: function dorecipe */
bool
dorecipe(Node *node)
{
    // iterators
    Arc *a;
    Node *n;
    Word *w;
    Symtab *s;
    /*s: [[dorecipe()]] other locals */
    Arc *aa = nil; // arc with recipe
    Rule *r = nil; // rule with recipe
    /*x: [[dorecipe()]] other locals */
    Word head, ahead; // out of date targets,   all targets
    /*x: [[dorecipe()]] other locals */
    Word lp, ln; // all prereqs, prereqs making target out of date
    /*x: [[dorecipe()]] other locals */
    bool did = false;
    /*x: [[dorecipe()]] other locals */
    Word *ww, *aw;
    char buf[BIGBLOCK];
    /*x: [[dorecipe()]] other locals */
    char cwd[256];
    /*e: [[dorecipe()]] other locals */

    /*
     *   pick up the rule
     */
    for(a = node->prereqs; a; a = a->next)
        if(*a->r->recipe) {
            aa = a;
            r = a->r;
        }
    /*s: [[dorecipe()]] if no recipe found */
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
        // else
        /*s: [[dorecipe()]] when no recipe found, if archive name */
        if(strchr(node->name, '(') && node->time == 0)
            MADESET(node, MADE);
        /*e: [[dorecipe()]] when no recipe found, if archive name */
        else
            update(false, node);

        /*s: [[dorecipe()]] when no recipe found, if tflag */
        if(tflag){
            if(!(node->flags&VIRTUAL))
                touch(node->name);
            else if(explain)
                Bprint(&bout, "no touch of virtual '%s'\n", node->name);
        }
        /*e: [[dorecipe()]] when no recipe found, if tflag */
        return did;
    }
    /*e: [[dorecipe()]] if no recipe found */
    // else

    /*
     *   build the node list
     */
    node->next = nil;
    head.next = ahead.next = nil;
    /*s: [[dorecipe()]] build lists of targets and node list */
    ww = &head;
    aw = &ahead;
    /*s: [[dorecipe()]] if regexp rule */
    if(r->attr&REGEXP){
        ww->next = newword(node->name);
        aw->next = newword(node->name);
    }
    /*e: [[dorecipe()]] if regexp rule */
    else {
        for(w = r->alltargets; w; w = w->next){
            if(r->attr&META)
                subst(aa->stem, w->s, buf, sizeof(buf));
            else
                strecpy(buf, buf + sizeof buf - 1, w->s);

            aw->next = newword(buf);
            aw = aw->next;

            s = symlook(buf, S_NODE, nil);
            if(s == nil)
                continue;	/* not a node we are interested in */
            // else
            n = s->u.ptr;

            /*s: [[dorecipe()]] update list of outdated targets */
            if(!aflag && n->time) {
                for(a = n->prereqs; a; a = a->next)
                    if(a->n && outofdate(n, a, false))
                        break;
                // no out of date arc, node does not need to be regenerated
                if(a == nil)
                    continue; 
                // else, find an outdated arc for node of target
            }
            ww->next = newword(buf);
            ww = ww->next;
            /*e: [[dorecipe()]] update list of outdated targets */

            if(n == node) continue;

            n->next = node->next;
            node->next = n;
        }
    }
    /*e: [[dorecipe()]] build lists of targets and node list */
    /*s: [[dorecipe()]] return if one target not READY */
    for(n = node; n; n = n->next)
        if(!(n->flags&READY))
            return did;
    /*e: [[dorecipe()]] return if one target not READY */

    /*
     *   gather the params for the job
     */
    lp.next = ln.next = nil;
    for(n = node; n; n = n->next){
        /*s: [[dorecipe()]] build lists of prerequisites */
        for(a = n->prereqs; a; a = a->next){
            if(a->n){
                addw(&lp, a->n->name);
                if(outofdate(n, a, false)){
                    addw(&ln, a->n->name);
                    /*s: [[dorecipe()]] explain when found arc [[a]] making target [[n]] out of date */
                    if(explain)
                        fprint(STDOUT, "%s(%ld) < %s(%ld)\n",
                            n->name, n->time, a->n->name, a->n->time);
                    /*e: [[dorecipe()]] explain when found arc [[a]] making target [[n]] out of date */
                }
            } else {
                /*s: [[dorecipe()]] explain when found target [[n]] with no prerequisite */
                if(explain)
                    fprint(STDOUT, "%s has no prerequisites\n", n->name);
                /*e: [[dorecipe()]] explain when found target [[n]] with no prerequisite */
            }
        }
        /*e: [[dorecipe()]] build lists of prerequisites */
        MADESET(n, BEINGMADE);
    }

    // run the job
    run(newjob(r, node, aa->stem, aa->match, 
               lp.next, ln.next, 
               head.next, ahead.next));
    return true;
}
/*e: function dorecipe */

/*e: mk/recipe.c */
