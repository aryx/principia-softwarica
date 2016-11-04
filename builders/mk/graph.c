/*s: mk/graph.c */
#include	"mk.h"

static Node *applyrules(char *, char *);
static void togo(Node *);
static int vacuous(Node *);
Arc* newarc(Node *n, Rule *r, char *stem, Resub *match);

static Node *newnode(char *);
static void trace(char *, Arc *);
static void cyclechk(Node *);
static void ambiguous(Node *);
static void attribute(Node *);

/*s: global nreps */
int nreps = 1;
/*e: global nreps */

/*s: function graph */
Node*
graph(char *target)
{
    Node *root;
    /*s: [[graph()]] other locals */
    // map<ruleid, int>
    char *cnt;
    /*e: [[graph()]] other locals */

    /*s: [[graph()]] set cnt for infinite rule detection */
    cnt = rulecnt();
    /*e: [[graph()]] set cnt for infinite rule detection */
    root = applyrules(target, cnt);
    /*s: [[graph()]] free cnt */
    free(cnt);
    /*e: [[graph()]] free cnt */

    /*s: [[graph()]] checking the graph */
    cyclechk(root);

    /*s: [[graph()]] before [[ambiguous()]] */
    root->flags |= PROBABLE;	/* make sure it doesn't get deleted */
    /*x: [[graph()]] before [[ambiguous()]] */
    vacuous(root);
    /*e: [[graph()]] before [[ambiguous()]] */
    ambiguous(root);
    /*e: [[graph()]] checking the graph */
    /*s: [[graph()]] propagate attributes */
    // propagate attributes in rules to their node
    attribute(root);
    /*e: [[graph()]] propagate attributes */

    return root;
}
/*e: function graph */

/*s: function applyrules */
static Node*
applyrules(char *target, char *cnt)
{
    Node *node;
    // list<ref<Arc> (next = Arc.next, last = lasta)
    Arc head;
    // ref<Arc>
    Arc *lasta = &head;
    /*s: [[applyrules]] other locals */
    Symtab *sym;
    Rule *r;
    Word *pre;
    Arc *arc;
    /*x: [[applyrules]] other locals */
    char stem[NAMEBLOCK];
    /*x: [[applyrules]] other locals */
    char buf[NAMEBLOCK];
    /*x: [[applyrules]] other locals */
    Resub rmatch[NREGEXP];
    /*e: [[applyrules]] other locals */

    /*s: [[applyrules]] debug */
    if(DEBUG(D_TRACE)) 
        print("applyrules(%lux='%s')\n", target, target);
    /*e: [[applyrules]] debug */
    /*s: [[applyrules]] check node cache if target is already there */
    sym = symlook(target, S_NODE, nil);
    if(sym)
        return sym->u.ptr;
    /*e: [[applyrules]] check node cache if target is already there */
    // else

    target = strdup(target);
    node = newnode(target); // calls timeof() internally
    head.next = nil;
    /*s: [[applyrules]] other initializations */
    memset((char*)rmatch, 0, sizeof(rmatch));
    /*e: [[applyrules]] other initializations */

    // apply regular rules with target as a head (modifies lasta)
    /*s: [[applyrules()]] apply regular rules */
    sym = symlook(target, S_TARGET, nil);
    r = sym? sym->u.ptr : nil;
    for(; r; r = r->chain){
        /*s: [[applyrules()]] skip this rule and continue if some conditions */
        if(empty_recipe(r) && empty_prereqs(r))
              continue;	/* no effect; ignore */
        /*e: [[applyrules()]] skip this rule and continue if some conditions */
        /*s: [[applyrules()]] infinite rule detection part1 */
        if(cnt[r->rule] >= nreps) 
            continue;

        cnt[r->rule]++;
        /*e: [[applyrules()]] infinite rule detection part1 */
        /*s: [[applyrules()]] when found a regular rule for target [[node]], set flags */
        node->flags |= PROBABLE;
        /*e: [[applyrules()]] when found a regular rule for target [[node]], set flags */

        /*s: [[applyrules()]] if no prerequistes in rule r */
        // no prerequistes, a leaf, still create fake arc
        if(empty_prereqs(r)) {
            arc = newarc((Node *)nil, r, "", rmatch);
            // add_list(head, arc)
            lasta->next = arc;
            lasta = lasta->next;
        } 
        /*e: [[applyrules()]] if no prerequistes in rule r */
        else
            for(pre = r->prereqs; pre; pre = pre->next){
                // recursive call!
                arc = newarc(applyrules(pre->s, cnt), r, "", rmatch);
                // add_list(head, arc)
                lasta->next = arc;
                lasta = lasta->next;
        }
        /*s: [[applyrules()]] infinite rule detection part2 */
        cnt[r->rule]--;
        /*e: [[applyrules()]] infinite rule detection part2 */
    }
    /*e: [[applyrules()]] apply regular rules */

    // apply meta rules (modifies lasta)
    /*s: [[applyrules()]] apply meta rules */
    for(r = metarules; r; r = r->next){
        /*s: [[applyrules()]] skip this meta rule and continue if some conditions */
        if(empty_recipe(r) && empty_prereqs(r)) 
            continue;	/* no effect; ignore */
        /*x: [[applyrules()]] skip this meta rule and continue if some conditions */
        if ((r->attr&NOVIRT) && lasta != &head && (lasta->r->attr&VIR))
            continue;
        /*e: [[applyrules()]] skip this meta rule and continue if some conditions */
        /*s: [[applyrules()]] if regexp rule then continue if some conditions */
        if(r->attr&REGEXP){
            stem[0] = '\0';
            memset((char*)rmatch, 0, sizeof(rmatch));
            patrule = r;
            if(regexec(r->pat, node->name, rmatch, NREGEXP) == 0)
                continue;
        }
        /*e: [[applyrules()]] if regexp rule then continue if some conditions */
        else {
            if(match(node->name, r->target, stem)) {
                /*s: [[applyrules()]] infinite rule detection part1 */
                if(cnt[r->rule] >= nreps) 
                    continue;

                cnt[r->rule]++;
                /*e: [[applyrules()]] infinite rule detection part1 */

                /*s: [[applyrules()]] if no prerequistes in meta rule r */
                if(empty_prereqs(r)) {
                    arc = newarc((Node *)nil, r, stem, rmatch);
                    // add_list(head, arc)
                    lasta->next = arc;
                    lasta = lasta->next;
                } 
                /*e: [[applyrules()]] if no prerequistes in meta rule r */
                else
                    for(pre = r->prereqs; pre; pre = pre->next) {
                        /*s: [[applyrules()]] if regexp rule, adjust buf and rmatch */
                        if(r->attr&REGEXP)
                            regsub(pre->s, buf, sizeof(buf), rmatch, NREGEXP);
                        /*e: [[applyrules()]] if regexp rule, adjust buf and rmatch */
                        else
                            subst(stem, pre->s, buf, sizeof(buf));
                        // recursive call!
                        arc = newarc(applyrules(buf, cnt), r, stem, rmatch);
                        // add_list(head, arc)
                        lasta->next = arc;
                        lasta = lasta->next;
                    }
                 /*s: [[applyrules()]] infinite rule detection part2 */
                 cnt[r->rule]--;
                 /*e: [[applyrules()]] infinite rule detection part2 */
           }
        }
    }
    /*e: [[applyrules()]] apply meta rules */

    node->arcs = head.next;

    return node;
}
/*e: function applyrules */

/*s: function nrep */
void
nrep(void)
{
    Symtab *sym;
    Word *w;

    sym = symlook("NREP", S_VAR, nil);
    if(sym){
        w = sym->u.ptr;
        if (w && w->s && *w->s)
            nreps = atoi(w->s);
    }
    if(nreps < 1)
        nreps = 1;
    /*s: [[nrep()]] if DEBUG(D_GRAPH) */
    if(DEBUG(D_GRAPH))
        Bprint(&bout, "nreps = %d\n", nreps);
    /*e: [[nrep()]] if DEBUG(D_GRAPH) */
}
/*e: function nrep */

/*s: function togo */
static void
togo(Node *node)
{
    Arc *a; 
    Arc *preva = nil;

    /* delete them now */
    for(a = node->arcs; a; preva = a, a = a->next)
        if(a->remove){

            //remove_list(a, node->arcs)
            if(a == node->arcs)
                node->arcs = a->next;
            else {
                preva->next = a->next;
                a = preva;
            }
        }
}
/*e: function togo */

/*s: function vacuous */
static bool
vacuous(Node *node)
{
    Arc *a;
    bool vac = !(node->flags&PROBABLE);
    /*s: [[vacuous()]] other locals */
    Arc *a2;
    /*e: [[vacuous()]] other locals */

    if(node->flags&READY)
        return node->flags&VACUOUS;

    node->flags |= READY;

    for(a = node->arcs; a; a = a->next)
        if(a->n && vacuous(a->n) && (a->r->attr&META))
            a->remove = true;
        else
            vac = false;
    /*s: [[vacuous]] possibly undelete some arcs */
    /* if a rule generated arcs that DON'T go; no others from that rule go */
    for(a = node->arcs; a; a = a->next)
        if(!(a->remove))
            for(a2 = node->arcs; a2; a2 = a2->next)
                if((a2->remove) && (a2->r == a->r)){
                    a2->remove = false;
                }
    /*e: [[vacuous]] possibly undelete some arcs */

    togo(node);
    if(vac) {
        node->flags |= VACUOUS;
    }
    return vac;
}
/*e: function vacuous */

/*s: constructor newnode */
static Node*
newnode(char *name)
{
    Node *node;

    node = (Node *)Malloc(sizeof(Node));
    /*s: [[newnode()]] update node cache */
    symlook(name, S_NODE, (void *)node);
    /*e: [[newnode()]] update node cache */

    node->name = name;
    // call to timeof()! 
    node->time = timeof(name, false);
    node->flags = 0;
    /*s: [[newnode()]] adjust flags of node */
    if(node->time)
        node->flags = PROBABLE;
    /*e: [[newnode()]] adjust flags of node */

    node->arcs = nil;
    node->next = nil;
    /*s: [[newnode()]] debug */
    if(DEBUG(D_TRACE)) 
        print("newnode(%s), time = %d\n", name, node->time);
    /*e: [[newnode()]] debug */
    return node;
}
/*e: constructor newnode */

/*s: constructor newarc */
Arc*
newarc(Node *n, Rule *r, char *stem, Resub *match)
{
    Arc *a;

    a = (Arc *)Malloc(sizeof(Arc));
    a->n = n;
    a->r = r;
    a->stem = strdup(stem);

    a->next = nil;
    /*s: [[newarc()]] set other fields */
    a->remove = false;
    /*x: [[newarc()]] set other fields */
    rcopy(a->match, match, NREGEXP);
    /*x: [[newarc()]] set other fields */
    a->prog = r->prog;
    /*e: [[newarc()]] set other fields */
    return a;
}
/*e: constructor newarc */


/*s: function trace */
static void
trace(char *s, Arc *a)
{
    fprint(STDERR, "\t%s", s);
    while(a){
        fprint(STDERR, " <-(%s:%d)- %s", a->r->file, a->r->line,
            a->n? a->n->name:"");
        if(a->n){
            for(a = a->n->arcs; a; a = a->next)
                if(*a->r->recipe) break;
        } else
            a = nil;
    }
    fprint(STDERR, "\n");
}
/*e: function trace */

/*s: function cyclechk */
static void
cyclechk(Node *n)
{
    Arc *a;

    if((n->flags&CYCLE)){
        fprint(STDERR, "mk: cycle in graph detected at target %s\n", n->name);
        Exit();
    }
    n->flags |= CYCLE;
    for(a = n->arcs; a; a = a->next)
        if(a->n)
            cyclechk(a->n);
    n->flags &= ~CYCLE;
}
/*e: function cyclechk */

/*s: function ambiguous */
static void
ambiguous(Node *n)
{
    Arc *a;
    Rule *master_rule = nil;
    Arc *master_arc = nil;
    bool error_reported = false;

    for(a = n->arcs; a; a = a->next){
        // recurse
        if(a->n)
            ambiguous(a->n);

        // arcs without any recipe do not generate ambiguity
        if(empty_recipe(a->r)) 
            continue;
        // else

        // first arc with a recipe (so no ambiguity)
        if(master_rule == nil) {
            master_rule = a->r;
            master_arc = a;
        }
        else{
            /*s: [[ambiguous()]] give priority to simple rules over meta rules */
            if(master_rule->recipe != a->r->recipe){
                if((master_rule->attr&META) && !(a->r->attr&META)){
                    master_arc->remove = true;
                    master_rule = a->r;
                    master_arc = a;
                } else if(!(master_rule->attr&META) && (a->r->attr&META)){
                    a->remove = true;
                    continue;
                }
            }
            /*e: [[ambiguous()]] give priority to simple rules over meta rules */
            if(master_rule->recipe != a->r->recipe){
                if(!error_reported){
                    fprint(STDERR, "mk: ambiguous recipes for %s:\n", n->name);
                    error_reported = true;
                    trace(n->name, master_arc);
                }
                trace(n->name, a);
            }
        }
    }
    if(error_reported)
        Exit();
    /*s: [[ambiguous()]] get rid of all skipped arcs */
    togo(n);
    /*e: [[ambiguous()]] get rid of all skipped arcs */
}
/*e: function ambiguous */

/*s: function attribute */
static void
attribute(Node *n)
{
    Arc *a;

    for(a = n->arcs; a; a = a->next){
        /*s: [[attribute()]] propagate rule attribute to node cases */
        if(a->r->attr&VIR)
            n->flags |= VIRTUAL;
        /*x: [[attribute()]] propagate rule attribute to node cases */
        if(a->r->attr&DEL)
            n->flags |= DELETE;
        /*x: [[attribute()]] propagate rule attribute to node cases */
        if(a->r->attr&NOREC)
            n->flags |= NORECIPE;
        /*e: [[attribute()]] propagate rule attribute to node cases */
        // recurse
        if(a->n)
            attribute(a->n);
    }
    /*s: [[attribute()]] if virtual node */
    if(n->flags&VIRTUAL)
        n->time = 0;
    /*e: [[attribute()]] if virtual node */
}
/*e: function attribute */
/*e: mk/graph.c */
