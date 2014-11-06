/*s: mk/mk.c */
#include	"mk.h"

void clrmade(Node*);
int	 work(Node*, Node*, Arc*);

/*s: global runerrs */
int runerrs;
/*e: global runerrs */

/*s: function mk */
void
mk(char *target)
{
    Node *node;
    bool did = false;

    /*s: [[mk()]] initialisation */
    nproc();	/* it can be updated dynamically */
    nrep();		/* it can be updated dynamically */
    runerrs = 0;
    /*e: [[mk()]] initialisation */
    node = graph(target);
    /*s: [[main()]] if DEBUG(D_GRAPH) */
    if(DEBUG(D_GRAPH)){
        dumpn("new target\n", node);
        Bflush(&bout);
    }
    /*e: [[main()]] if DEBUG(D_GRAPH) */
    clrmade(node);

    while(node->flags&NOTMADE){
        if(work(node, (Node *)nil, (Arc *)nil))
            did = true;	/* found something to do */
        else {
            if(waitup(1, (int *)nil) > 0){
                if(node->flags&(NOTMADE|BEINGMADE)){
                    assert(/*must be run errors*/ runerrs);
                    break;	/* nothing more waiting */
                }
            }
        }
    }
    if(node->flags&BEINGMADE)
        waitup(-1, (int *)nil);

    while(jobs)
        waitup(-2, (int *)nil);

    assert(/*target didnt get done*/ runerrs || (node->flags&MADE));
    if(did == false)
        Bprint(&bout, "mk: '%s' is up to date\n", node->name);
}
/*e: function mk */

/*s: function clrmade */
void
clrmade(Node *n)
{
    Arc *a;

    /*s: [[clrmade()]] n->flags pretend adjustments */
    n->flags &= ~(CANPRETEND|PRETENDING);
    if(strchr(n->name, '(') ==0 || n->time)
        n->flags |= CANPRETEND;
    /*e: [[clrmade()]] n->flags pretend adjustments */
    MADESET(n, NOTMADE);
    for(a = n->prereqs; a; a = a->next)
        if(a->n)
            clrmade(a->n);
}
/*e: function clrmade */

/*s: function unpretend */
static void
unpretend(Node *n)
{
    MADESET(n, NOTMADE);
    n->flags &= ~(CANPRETEND|PRETENDING);
    n->time = 0;
}
/*e: function unpretend */

/*s: function work */
bool
work(Node *node,   Node *p, Arc *parc)
{
    /*s: [[work()]] locals */
    bool did = false;
    /*x: [[work()]] locals */
    bool weoutofdate;
    bool ready;
    Arc *a;
    Arc *ra;
    /*x: [[work()]] locals */
    char cwd[256];
    /*e: [[work()]] locals */

    if(node->flags&BEINGMADE)
        return did;

    /*s: [[work()]] possibly unpretending node */
    if((node->flags&MADE) && (node->flags&PRETENDING) && p
        && outofdate(p, parc, 0)){
        if(explain)
            fprint(1, "unpretending %s(%lud) because %s is out of date(%lud)\n",
                node->name, node->time, p->name, p->time);
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
            return did;
    }
    /*e: [[work()]] possibly unpretending node */
    /*s: [[work()]] no prerequisite special case */
    /* consider no prerequisite case */
    if(node->prereqs == nil){
        if(node->time == 0){
            if(getwd(cwd, sizeof cwd))
                fprint(STDERR, "mk: don't know how to make '%s' in directory %s\n", node->name, cwd);
            else
                fprint(STDERR, "mk: don't know how to make '%s'\n", node->name);

            if(kflag){
                node->flags |= BEINGMADE;
                runerrs++;
            } else
                Exit();
        } else
            MADESET(node, MADE);
        return did;
    }
    /*e: [[work()]] no prerequisite special case */

    /*
     *   now see if we are out of date or what
     */
    ready = true;
    weoutofdate = aflag;
    /*s: [[work()]] check if node out of date with prerequisites, recursively */
    ra = nil;
    for(a = node->prereqs; a; a = a->next)
        if(a->n){
            // recursive call! go in depth
            did = work(a->n, node, a) || did;
            if(a->n->flags&(NOTMADE|BEINGMADE))
                ready = false;
            if(outofdate(node, a, 0)){
                weoutofdate = true;
                if((ra == nil) || (ra->n == nil) || (ra->n->time < a->n->time))
                    ra = a;
            }
        } else {
            if(node->time == 0){
                weoutofdate = true;
                if(ra == nil)
                    ra = a;
            }
        }
    /*e: [[work()]] check if node out of date with prerequisites, recursively */

    if(ready == false)	/* can't do anything now */
        return did;
    if(weoutofdate == false){
        MADESET(node, MADE);
        return did;
    }
    /*s: [[work()]] possibly pretending node */
    /*
     *   can we pretend to be made?
     */
    if((iflag == false) && (node->time == 0) 
            && (node->flags&(PRETENDING|CANPRETEND))
            && p && ra->n && !outofdate(p, ra, 0)){
        node->flags &= ~CANPRETEND;
        MADESET(node, MADE);
        if(explain && ((node->flags&PRETENDING) == 0))
            fprint(1, "pretending %s has time %lud\n", node->name, node->time);
        node->flags |= PRETENDING;
        return did;
    }
    /*
     *   node is out of date and we REALLY do have to do something.
     *   quickly rescan for pretenders
     */
    for(a = node->prereqs; a; a = a->next)
        if(a->n && (a->n->flags&PRETENDING)){
            if(explain)
                Bprint(&bout, "unpretending %s because of %s because of %s\n",
                a->n->name, node->name, 
                ra->n? ra->n->name : "rule with no prerequisites");

            unpretend(a->n);
            did = work(a->n, node, a) || did;
            ready = false;
        }
    if(ready == false)/* try later unless nothing has happened for -k's sake */
        return did || work(node, p, parc);
    /*e: [[work()]] possibly pretending node */

    did = dorecipe(node) || did;
    return did;
}
/*e: function work */

/*s: function update */
void
update(bool fake, Node *node)
{
    Arc *a;

    MADESET(node, fake? BEINGMADE : MADE);

    if(((node->flags&VIRTUAL) == 0) && (access(node->name, 0) == 0)){
        node->time = timeof(node->name, true);
        /*s: [[update()]] unpretend node */
        node->flags &= ~(CANPRETEND|PRETENDING);
        /*e: [[update()]] unpretend node */
        for(a = node->prereqs; a; a = a->next)
            if(a->prog)
                outofdate(node, a, 1);
    } else {
        node->time = 1;
        for(a = node->prereqs; a; a = a->next)
            if(a->n && outofdate(node, a, 1))
                node->time = a->n->time;
    }
}
/*e: function update */

/*s: function pcmp */
static int
pcmp(char *prog, char *p, char *q)
{
    char buf[3*NAMEBLOCK];
    int pid;

    Bflush(&bout);
    snprint(buf, sizeof buf, "%s '%s' '%s'\n", prog, p, q);
    pid = pipecmd(buf, 0, 0);
    while(waitup(-3, &pid) >= 0)
        ;
    return (pid? 2:1);
}
/*e: function pcmp */

/*s: function outofdate */
bool
outofdate(Node *node, Arc *arc, int eval)
{
    char buf[3*NAMEBLOCK], *str;
    Symtab *sym;
    int ret;

    str = 0;
    if(arc->prog){
        snprint(buf, sizeof buf, "%s%c%s", node->name, 0377,
            arc->n->name);
        sym = symlook(buf, S_OUTOFDATE, 0);
        if(sym == 0 || eval){
            if(sym == 0)
                str = strdup(buf);
            ret = pcmp(arc->prog, node->name, arc->n->name);
            if(sym)
                sym->u.value = ret;
            else
                symlook(str, S_OUTOFDATE, (void *)ret);
        } else
            ret = sym->u.value;
        return (ret-1);
    } else if(strchr(arc->n->name, '(') && arc->n->time == 0)  /* missing archive member */
        return true;
    else
        /*
         * Treat equal times as out-of-date.
         * It's a race, and the safer option is to do
         * extra building rather than not enough.
         */
        return node->time <= arc->n->time;
}
/*e: function outofdate */
/*e: mk/mk.c */
