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
    /*s: [[graph()]] set root flags before [[vacuous()]] */
    root->flags |= PROBABLE;	/* make sure it doesn't get deleted */
    /*e: [[graph()]] set root flags before [[vacuous()]] */
    vacuous(root);
    ambiguous(root);
    /*e: [[graph()]] checking the graph */

    // propagate attributes in rules to their node
    attribute(root);

    return root;
}
/*e: function graph */

/*s: function applyrules */
static Node*
applyrules(char *target, char *cnt)
{
    Node *node;
    Arc head;
    Arc *a = &head;
    /*s: [[applyrules]] other locals */
    Symtab *sym;
    Rule *r;
    Word *w;
    /*x: [[applyrules]] other locals */
    char buf[NAMEBLOCK];
    char stem[NAMEBLOCK];
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
    // calls timeof()
    node = newnode(target);

    head.n = nil;
    head.next = nil;

    /*s: [[applyrules]] set rmatch */
    memset((char*)rmatch, 0, sizeof(rmatch));
    /*e: [[applyrules]] set rmatch */

    // apply regular rules with target as a head (modify node, head, a)
    /*s: [[applyrules()]] apply regular rules */
    sym = symlook(target, S_TARGET, nil);
    for(r = (sym? sym->u.ptr : nil); r; r = r->chain){
        /*s: [[applyrules()]] skip this rule and continue if some conditions */
        if(r->attr&META) continue;
        if(strcmp(target, r->target)) continue; // how can happen??
        if((!r->recipe || !*r->recipe)
           && (!r->tail || !r->tail->s || !*r->tail->s)) 
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
        if(!r->tail || !r->tail->s || !*r->tail->s) {
            a->next = newarc((Node *)nil, r, "", rmatch);
            a = a->next;
        } 
        /*e: [[applyrules()]] if no prerequistes in rule r */
        else
            for(w = r->tail; w; w = w->next){
                // recursive call!
                a->next = newarc(applyrules(w->s, cnt), r, "", rmatch);
                a = a->next;
        }
        /*s: [[applyrules()]] infinite rule detection part2 */
        cnt[r->rule]--;
        /*e: [[applyrules()]] infinite rule detection part2 */
        head.n = node;
    }
    /*e: [[applyrules()]] apply regular rules */

    // apply meta rules
    /*s: [[applyrules()]] apply meta rules */
    for(r = metarules; r; r = r->next){
        /*s: [[applyrules()]] skip this meta rule and continue if some conditions */
        if((!r->recipe || !*r->recipe) 
           && (!r->tail || !r->tail->s || !*r->tail->s)) 
            continue;	/* no effect; ignore */
        /*x: [[applyrules()]] skip this meta rule and continue if some conditions */
        if ((r->attr&NOVIRT) && a != &head && (a->r->attr&VIR))
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
            if(!match(node->name, r->target, stem)) continue;
        }
        /*s: [[applyrules()]] infinite rule detection part1 */
        if(cnt[r->rule] >= nreps) 
            continue;
        cnt[r->rule]++;
        /*e: [[applyrules()]] infinite rule detection part1 */

        /*s: [[applyrules()]] if no prerequistes in meta rule r */
        if(!r->tail || !r->tail->s || !*r->tail->s) {
            a->next = newarc((Node *)nil, r, stem, rmatch);
            a = a->next;
        } 
        /*e: [[applyrules()]] if no prerequistes in meta rule r */
        else
            for(w = r->tail; w; w = w->next){
                /*s: [[applyrules()]] if regexp rule, adjust buf and rmatch */
                if(r->attr&REGEXP)
                    regsub(w->s, buf, sizeof(buf), rmatch, NREGEXP);
                /*e: [[applyrules()]] if regexp rule, adjust buf and rmatch */
                else
                    subst(stem, w->s, buf, sizeof(buf));
                // recursive call!
                a->next = newarc(applyrules(buf, cnt), r, stem, rmatch);
                a = a->next;
            }
        /*s: [[applyrules()]] infinite rule detection part2 */
        cnt[r->rule]--;
        /*e: [[applyrules()]] infinite rule detection part2 */
    }
    /*e: [[applyrules()]] apply meta rules */

    // ???
    a->next = node->prereqs;
    node->prereqs = head.next;

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
    Arc *a, *la;

    /* delete them now */
    la = nil;
    for(a = node->prereqs; a; la = a, a = a->next)
        if(a->flag&TOGO){
            //remove_list(a, node->prereqs)
            if(a == node->prereqs)
                node->prereqs = a->next;
            else
                la->next = a->next, a = la;
        }
}
/*e: function togo */

/*s: function vacuous */
static bool
vacuous(Node *node)
{
    Arc *la, *a;
    bool vac = !(node->flags&PROBABLE);

    if(node->flags&READY)
        return node->flags&VACUOUS;
    node->flags |= READY;

    for(a = node->prereqs; a; a = a->next)
        if(a->n && vacuous(a->n) && (a->r->attr&META))
            a->flag |= TOGO;
        else
            vac = false;
    /*s: [[vacuous]] possibly undelete some arcs */
    /* if a rule generated arcs that DON'T go; no others from that rule go */
    for(a = node->prereqs; a; a = a->next)
        if(!(a->flag&TOGO))
            for(la = node->prereqs; la; la = la->next)
                if((la->flag&TOGO) && (la->r == a->r)){
                    la->flag &= ~TOGO;
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
    /*s: [[newnode()]] set flags of node */
    node->flags = (node->time? PROBABLE : 0);
    /*e: [[newnode()]] set flags of node */

    node->prereqs = nil;
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
    a->flag = 0;
    /*s: [[newarc()]] set other fields */
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
            for(a = a->n->prereqs; a; a = a->next)
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

    if((n->flags&CYCLE) && n->prereqs){
        fprint(STDERR, "mk: cycle in graph detected at target %s\n", n->name);
        Exit();
    }
    n->flags |= CYCLE;
    for(a = n->prereqs; a; a = a->next)
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
    Rule *r = nil;
    Arc *la = nil;
    bool bad = false;

    for(a = n->prereqs; a; a = a->next){
        // recurse
        if(a->n)
            ambiguous(a->n);

        // rules without any recipe do not generate ambiguity
        if(*a->r->recipe == '\0') continue;
        // else

        // first rule with recipe (so no ambiguity)
        if(r == nil) {
            r = a->r;
            la = a;
        }
        else{
            /*s: [[ambiguous()]] give priority to simple rules over meta rules */
            if(r->recipe != a->r->recipe){
                if((r->attr&META) && !(a->r->attr&META)){
                    la->flag |= TOGO;
                    r = a->r;
                    la = a;
                } else if(!(r->attr&META) && (a->r->attr&META)){
                    a->flag |= TOGO;
                    continue;
                }
            }
            /*e: [[ambiguous()]] give priority to simple rules over meta rules */
            if(r->recipe != a->r->recipe){
                if(!bad){
                    fprint(STDERR, "mk: ambiguous recipes for %s:\n", n->name);
                    bad = true;
                    trace(n->name, la);
                }
                trace(n->name, a);
            }
        }
    }
    if(bad)
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

    for(a = n->prereqs; a; a = a->next){
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
