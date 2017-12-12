/*s: mk/mk.c */
#include	"mk.h"

void clrmade(Node*);
void work(Node*, bool*, Node*, Arc*);

/*s: global [[runerrs]] */
int runerrs;
/*e: global [[runerrs]] */

/*s: function [[mk]] */
void
mk(char *target)
{
    Node *root;
    bool everdid = false;
    bool did = false;
    // enum<WaitupResult>
    int res;

    /*s: [[mk()]] initializations */
    nproc();	/* it can be updated dynamically */
    /*x: [[mk()]] initializations */
    nrep();		/* it can be updated dynamically */
    /*x: [[mk()]] initializations */
    runerrs = 0;
    /*e: [[mk()]] initializations */

    root = graph(target);
    /*s: [[mk()]] if [[DEBUG(D_GRAPH)]] */
    if(DEBUG(D_GRAPH)){
        dumpn("new target\n", root);
        Bflush(&bout);
    }
    /*e: [[mk()]] if [[DEBUG(D_GRAPH)]] */
    clrmade(root);

    while(root->flags&NOTMADE){
        did = false;
        work(root, &did,   (Node *)nil, (Arc *)nil);
        if(did)
            everdid = true;	/* found something to do */
        else {
            res = waitup(EMPTY_CHILDREN_IS_OK, (int *)nil);
            /*s: [[mk()]] if no child to waitup and root not MADE, possibly break */
            if(res > 0){
                if(root->flags&(NOTMADE|BEINGMADE)){
                    assert(/*must be run errors*/ runerrs);
                    break;	/* nothing more waiting */
                }
            }
            /*e: [[mk()]] if no child to waitup and root not MADE, possibly break */
        }
    }
    if(root->flags&BEINGMADE)
        waitup(EMPTY_CHILDREN_IS_ERROR, (int *)nil);

    /*s: [[mk()]] before returning, more [[waitup()]] if there was an error */
    while(jobs)
        waitup(EMPTY_CHILDREN_IS_ERROR2, (int *)nil);
    assert(/*target didnt get done*/ runerrs || (root->flags&MADE));
    /*e: [[mk()]] before returning, more [[waitup()]] if there was an error */
    if(!everdid)
        Bprint(&bout, "mk: '%s' is up to date\n", root->name);
    return;
}
/*e: function [[mk]] */

/*s: function [[clrmade]] */
void
clrmade(Node *n)
{
    Arc *a;

    /*s: [[clrmade()]] [[n->flags]] pretend adjustments */
    n->flags &= ~(CANPRETEND|PRETENDING);
    if(strchr(n->name, '(') == nil || n->time)
        n->flags |= CANPRETEND;
    /*e: [[clrmade()]] [[n->flags]] pretend adjustments */
    MADESET(n, NOTMADE);
    for(a = n->arcs; a; a = a->next)
        if(a->n)
            // recurse
            clrmade(a->n);
}
/*e: function [[clrmade]] */

/*s: function [[unpretend]] */
static void
unpretend(Node *n)
{
    MADESET(n, NOTMADE);
    n->flags &= ~(CANPRETEND|PRETENDING);
    n->time = 0;
}
/*e: function [[unpretend]] */

/*s: function [[work]] */
void
work(Node *node, bool *did,   Node *parent_node, Arc *parent_arc)
{
    /*s: [[work()]] locals */
    char cwd[256];
    /*x: [[work()]] locals */
    bool weoutofdate = false;
    bool ready = true;
    /*x: [[work()]] locals */
    Arc *a;
    /*x: [[work()]] locals */
    Arc *ra = nil;
    /*e: [[work()]] locals */

    /*s: [[work()]] debug */
    if(DEBUG(D_TRACE))
        print("work(%s) flags=0x%x time=%lud\n", node->name, node->flags, node->time);
    /*e: [[work()]] debug */
    if(node->flags&BEINGMADE)
        return;
    /*s: [[work()]] possibly unpretending node */
    if((node->flags&MADE) && (node->flags&PRETENDING) && parent_node
        && outofdate(parent_node, parent_arc, false)){
        if(explain)
            fprint(STDOUT, "unpretending %s(%lud) because %s is out of date(%lud)\n",
                node->name, node->time, parent_node->name, parent_node->time);
        unpretend(node);
    }
    /*
     *   have a look if we are pretending in case
     *   someone has been unpretended out from underneath us
     */
    if(node->flags&MADE){
        if(node->flags&PRETENDING){
            node->time = 0;
        }else
            return;
    }
    /*e: [[work()]] possibly unpretending node */

    if(node->arcs == nil){
        /*s: [[work()]] no arcs, a leaf */
        /* consider no prerequisite case */
        if(node->time == 0){
            /*s: [[work()]] print error when inexistent file without prerequisites */
            if(getwd(cwd, sizeof cwd))
                fprint(STDERR, "mk: don't know how to make '%s' in directory %s\n", node->name, cwd);
            else
                fprint(STDERR, "mk: don't know how to make '%s'\n", node->name);
            /*s: [[work()]] when inexistent target without prerequisites, if kflag */
            if(kflag){
                node->flags |= BEINGMADE;
                runerrs++;
            }
            /*e: [[work()]] when inexistent target without prerequisites, if kflag */
            else
                Exit();
            /*e: [[work()]] print error when inexistent file without prerequisites */
        } else
            MADESET(node, MADE);
        /*e: [[work()]] no arcs, a leaf */
    } else {
        /*s: [[work()]] some arcs, a node */
        /*s: [[work()]] adjust weoutofdate if aflag */
        if(aflag)
            weoutofdate = true;
        /*e: [[work()]] adjust weoutofdate if aflag */
        /*
         *   now see if we are out of date or what
         */
        for(a = node->arcs; a; a = a->next) {
            if(a->n){
                // recursive call! go in depth
                work(a->n, did,  node, a);

                if(a->n->flags&(NOTMADE|BEINGMADE))
                    ready = false;
                if(outofdate(node, a, false)){
                    weoutofdate = true;
                    /*s: [[work()]] update [[ra]] when outofdate [[node]] with arc [[a]] */
                    if((ra == nil) || (ra->n == nil) || (ra->n->time < a->n->time))
                        ra = a;
                    /*e: [[work()]] update [[ra]] when outofdate [[node]] with arc [[a]] */
                }
            } else {
                if(node->time == 0){
                    weoutofdate = true;
                    /*s: [[work()]] update [[ra]] when no dest in arc and no src */
                    if(ra == nil)
                        ra = a;
                    /*e: [[work()]] update [[ra]] when no dest in arc and no src */
                }
            }
        }

        if(!ready)	/* can't do anything now */
            return;
        if(!weoutofdate){
            MADESET(node, MADE);
            return;
        }
        /*s: [[work()]] possibly pretending node */
        /*
         *   can we pretend to be made?
         */
        if((!iflag) && (node->time == 0) 
                && (node->flags&(PRETENDING|CANPRETEND))
                && parent_node && ra->n && !outofdate(parent_node, ra, false)){
            node->flags &= ~CANPRETEND;
            MADESET(node, MADE);
            if(explain && ((node->flags&PRETENDING) == 0))
                fprint(STDOUT, "pretending %s has time %lud\n", node->name, node->time);
            node->flags |= PRETENDING;
            return;
        }
        /*
         *   node is out of date and we REALLY do have to do something.
         *   quickly rescan for pretenders
         */
        for(a = node->arcs; a; a = a->next)
            if(a->n && (a->n->flags&PRETENDING)){
                if(explain)
                    Bprint(&bout, "unpretending %s because of %s because of %s\n",
                    a->n->name, node->name, 
                    ra->n? ra->n->name : "rule with no prerequisites");

                unpretend(a->n);
                work(a->n, did, node, a);
                ready = false;
            }
        if(!ready) { /* try later unless nothing has happened for -k's sake */
            work(node, did, parent_node, parent_arc);
            return;
        }
        /*e: [[work()]] possibly pretending node */
        // else, out of date

        dorecipe(node, did);
        return;
        /*e: [[work()]] some arcs, a node */
    }
}
/*e: function [[work]] */

/*s: function [[update]] */
void
update(Node *node, bool fake)
{
    Arc *a;

    /*s: [[update()]] if fake */
    if(fake)
        MADESET(node, BEINGMADE);
    /*e: [[update()]] if fake */
    else
       MADESET(node, MADE);
    /*s: [[update()]] debug */
    if(DEBUG(D_TRACE))
        print("update(): node %s time=%lud flags=0x%x\n", node->name, node->time, node->flags);
    /*e: [[update()]] debug */

    /*s: [[update()]] if virtual node or inexistent file */
    if((node->flags&VIRTUAL) || (access(node->name, AEXIST) != OK_0)){
        node->time = 1;
        for(a = node->arcs; a; a = a->next)
            if(a->n && outofdate(node, a, true))
                node->time = a->n->time;
    }
    /*e: [[update()]] if virtual node or inexistent file */
    else {
        node->time = timeof(node->name, true);
        /*s: [[update()]] unpretend node */
        node->flags &= ~(CANPRETEND|PRETENDING);
        /*e: [[update()]] unpretend node */
        /*s: [[update()]] set outofdate prereqs if arc prog */
        for(a = node->arcs; a; a = a->next)
            if(a->prog)
                outofdate(node, a, true);
        /*e: [[update()]] set outofdate prereqs if arc prog */
    }
}
/*e: function [[update]] */

/*s: function [[pcmp]] */
static int
pcmp(char *prog, char *p, char *q)
{
    char buf[3*NAMEBLOCK];
    int pid;

    Bflush(&bout);
    snprint(buf, sizeof buf, "%s '%s' '%s'\n", prog, p, q);
    pid = pipecmd(buf, nil, nil);
    while(waitup(EMPTY_CHILDREN_IS_ERROR3, &pid) >= 0)
        ;
    return (pid? 2:1);
}
/*e: function [[pcmp]] */

/*s: function [[outofdate]] */
bool
outofdate(Node *node, Arc *arc, bool eval)
{
    /*s: [[outofdate()]] locals */
    char buf[3*NAMEBLOCK];
    char *str = nil;
    Symtab *sym;
    int ret;
    /*e: [[outofdate()]] locals */

    /*s: [[outofdate()]] if arc->prog */
    if(arc->prog){
        snprint(buf, sizeof buf, "%s%c%s", node->name, 0377,
            arc->n->name);
        sym = symlook(buf, S_OUTOFDATE, nil);
        if(sym == nil || eval){
            if(sym == nil)
                str = strdup(buf);
            ret = pcmp(arc->prog, node->name, arc->n->name);
            if(sym)
                sym->u.value = ret;
            else
                symlook(str, S_OUTOFDATE, (void *)ret);
        } else
            ret = sym->u.value;
        return (ret-1);
    }
    /*e: [[outofdate()]] if arc->prog */
    else 
     /*s: [[outofdate()]] if arc node is an archive member */
     if(strchr(arc->n->name, '(') && arc->n->time == 0)
        /* missing archive member */
        return true;
     /*e: [[outofdate()]] if arc node is an archive member */
     else
        /*
         * Treat equal times as out-of-date.
         * It's a race, and the safer option is to do
         * extra building rather than not enough.
         */
        return node->time < arc->n->time;
}
/*e: function [[outofdate]] */
/*e: mk/mk.c */
