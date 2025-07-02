/*s: mk/recipe.c */
#include	"mk.h"

/*s: constructor [[newjob]] */
Job*
newjob(Rule *r, Node *nlist, char *stem, char **match, 
       Word *allprereqs, Word *newprereqs, 
       Word *alltargets, Word *oldtargets)
{
    Job *j;

    j = (Job *)Malloc(sizeof(Job));
    j->r = r;
    j->n = nlist;
    j->p = allprereqs;
    j->t = oldtargets;

    j->stem = stem;
    j->match = match;

    /*s: [[newjob()]] setting other fields */
    j->at = alltargets;
    /*x: [[newjob()]] setting other fields */
    j->np = newprereqs;
    /*e: [[newjob()]] setting other fields */

    j->next = nil;
    return j;
}
/*e: constructor [[newjob]] */

/*s: function [[dorecipe]] */
void
dorecipe(Node *node, bool *did)
{
    // iterators
    Arc *a;
    Node *n;
    Word *w;
    Symtab *s; 
    /*s: [[dorecipe()]] other locals */
    Rule *master_rule = nil;
    Arc *master_arc = nil;
    /*x: [[dorecipe()]] other locals */
    // list<string> (last = last_alltargets)
    Word alltargets;
    /*x: [[dorecipe()]] other locals */
    // list<ref<Node>> (next = Node.next)
    Node *nlist = node;
    /*x: [[dorecipe()]] other locals */
    // list<string> (last = last_allprereqs)
    Word allprereqs;
    /*x: [[dorecipe()]] other locals */
    char cwd[256];
    /*x: [[dorecipe()]] other locals */
    Word *last_alltargets = &alltargets;
    char buf[BIGBLOCK];
    /*x: [[dorecipe()]] other locals */
    Word oldtargets;
    Word *last_oldtargets = &oldtargets;
    /*x: [[dorecipe()]] other locals */
    Word newprereqs; 
    /*e: [[dorecipe()]] other locals */

    /*
     *   pick up the master rule
     */
    for(a = node->arcs; a; a = a->next)
        if(!empty_recipe(a->r)) {
            master_rule = a->r;
            master_arc = a;
        }

    /*s: [[dorecipe()]] if no recipe found */
    /*
     *   no recipe? go to buggery!
     */
    if(master_rule == nil){
        /*s: [[dorecipe()]] when no recipe found, if virtual or norecipe node */
        if((node->flags&VIRTUAL) || (node->flags&NORECIPE)){
            /*s: [[dorecipe()]] when no recipe found, if archive name */
            if(strchr(node->name, '(') && node->time == 0)
                MADESET(node, MADE);
            /*e: [[dorecipe()]] when no recipe found, if archive name */
            else
                update(node, false);
            /*s: [[dorecipe()]] when no recipe found, if tflag */
            if(tflag){
                if(!(node->flags&VIRTUAL))
                    touch(node->name);
                else if(explain)
                    Bprint(&bout, "no touch of virtual '%s'\n", node->name);
            }
            /*e: [[dorecipe()]] when no recipe found, if tflag */
            //bugfix:
            return;
        }
        /*e: [[dorecipe()]] when no recipe found, if virtual or norecipe node */
        else {
            if(getwd(cwd, sizeof cwd))
                fprint(STDERR, "mk: no recipe to make '%s' in directory %s\n", 
                       node->name, cwd);
            else
                fprint(STDERR, "mk: no recipe to make '%s'\n", node->name);
            Exit();
        }
    }
    /*e: [[dorecipe()]] if no recipe found */
    // else

    /*
     *   build the node list
     */
    /*s: [[dorecipe()]] build lists of targets and node list */
    nlist->next = nil;
    alltargets.next = oldtargets.next = nil;
    /*s: [[dorecipe()]] if regexp rule */
    if(master_rule->attr&REGEXP){
        last_oldtargets->next = newword(node->name);
        last_alltargets->next = newword(node->name);
    }
    /*e: [[dorecipe()]] if regexp rule */
    else {
        for(w = master_rule->alltargets; w; w = w->next){
            if(master_rule->attr&META)
                subst(master_arc->stem, w->s, buf, sizeof(buf));
            else
                strecpy(buf, buf + sizeof buf - 1, w->s);

            //add_list(newword(buf), alltargets)
            last_alltargets->next = newword(buf);
            last_alltargets = last_alltargets->next;

            s = symlook(buf, S_NODE, nil);
            /*s: [[dorecipe()]] sanity check s */
            if(s == nil)
                continue;	/* not a node we are interested in */
            /*e: [[dorecipe()]] sanity check s */
            n = s->u.ptr;

            /*s: [[dorecipe()]] update list of outdated targets */
            if(!aflag && n->time) {
                for(a = n->arcs; a; a = a->next)
                    if(a->n && outofdate(n, a, false))
                        break;
                // no out of date arc, node does not need to be regenerated
                if(a == nil)
                    continue; 
                // else, find an outdated arc for node of target
            }
            last_oldtargets->next = newword(buf);
            last_oldtargets = last_oldtargets->next;
            /*e: [[dorecipe()]] update list of outdated targets */

            // add_set(n, nlist)
            if(n == node) 
                continue;
            n->next = nlist->next;
            nlist->next = n;
        }
    }
    /*e: [[dorecipe()]] build lists of targets and node list */

    /*
     *   gather the params for the job
     */
    allprereqs.next = newprereqs.next = nil;
    for(n = nlist; n; n = n->next){
        /*s: [[dorecipe()]] build lists of prerequisites */
        for(a = n->arcs; a; a = a->next){
            if(a->n){
                addw(&allprereqs, a->n->name);

                if(outofdate(n, a, false)){
                    /*s: [[dorecipe()]] when outofdate node, update list of newprereqs */
                    addw(&newprereqs, a->n->name);
                    /*e: [[dorecipe()]] when outofdate node, update list of newprereqs */
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
    run(newjob(master_rule, nlist, 
               master_arc->stem, master_arc->match, 
               allprereqs.next, newprereqs.next, 
               alltargets.next, oldtargets.next));
    *did = true; // finally
    return;
}
/*e: function [[dorecipe]] */

/*e: mk/recipe.c */
