/*s: mk/graph.c */
#include	"mk.h"

static Node *applyrules(char *, char *);
static void togo(Node *);
static int vacuous(Node *);
static Node *newnode(char *);
static void trace(char *, Arc *);
static void cyclechk(Node *);
static void ambiguous(Node *);
static void attribute(Node *);

/*s: function graph */
Node*
graph(char *target)
{
    Node *root;
    char *cnt;

    cnt = rulecnt();
    root = applyrules(target, cnt);
    free(cnt);

    cyclechk(root);

    root->flags |= PROBABLE;	/* make sure it doesn't get deleted */
    vacuous(root);
    ambiguous(root);

    attribute(root);

    return root;
}
/*e: function graph */

/*s: function applyrules */
static Node*
applyrules(char *target, char *cnt)
{
    /*s: [[applyrules]] locals */
    Symtab *sym;
    Node *node;
    Arc head;
    Arc *a = &head;
    /*x: [[applyrules]] locals */
    Rule *r;
    Word *w;
    /*x: [[applyrules]] locals */
    char buf[NAMEBLOCK];
    char stem[NAMEBLOCK];
    /*x: [[applyrules]] locals */
    Resub rmatch[NREGEXP];
    /*e: [[applyrules]] locals */
    
    if(DEBUG(D_TRACE)) 
        print("applyrules(%lux='%s')\n", target, target);

    sym = symlook(target, S_NODE, 0);
    if(sym)
        return sym->u.ptr;

    target = strdup(target);
    node = newnode(target);
    head.n = nil;
    head.next = nil;
    memset((char*)rmatch, 0, sizeof(rmatch));

    // apply regular rules with target as a head
    /*s: [[applyrules()]] apply regular rules */
    sym = symlook(target, S_TARGET, 0);
    for(r = (sym? sym->u.ptr : nil); r; r = r->chain){
        /*s: [[applyrules()]] skip this rule and continue if some conditions */
        if(r->attr&META) continue;
        if(strcmp(target, r->target)) continue; // how can happen??
        if((!r->recipe || !*r->recipe)
           && (!r->tail || !r->tail->s || !*r->tail->s)) 
              continue;	/* no effect; ignore */
        /*e: [[applyrules()]] skip this rule and continue if some conditions */
        if(cnt[r->rule] >= nreps) continue;

        cnt[r->rule]++;
        node->flags |= PROBABLE;

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
        cnt[r->rule]--;
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
        if(cnt[r->rule] >= nreps) continue;

        cnt[r->rule]++;

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
        cnt[r->rule]--;
    }
    /*e: [[applyrules()]] apply meta rules */

    a->next = node->prereqs;
    node->prereqs = head.next;
    return node;
}
/*e: function applyrules */

/*s: function togo */
static void
togo(Node *node)
{
    Arc *a, *la;

    /* delete them now */
    la = nil;
    for(a = node->prereqs; a; la = a, a = a->next)
        if(a->flag&TOGO){
            fprint(STDERR, "mk: vacuous arc found %s->%s\n", 
                     node->name, a->n->name);
            //delete(a, node->prereqs)
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

    /* if a rule generated arcs that DON'T go; no others from that rule go */
    for(a = node->prereqs; a; a = a->next)
        if((a->flag&TOGO) == 0)
            for(la = node->prereqs; la; la = la->next)
                if((la->flag&TOGO) && (la->r == a->r)){
                    la->flag &= ~TOGO;
                }
    togo(node);
    if(vac) {
        fprint(STDERR, "mk: vacuous node found %s\n", node->name);
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
    symlook(name, S_NODE, (void *)node);

    node->name = name;
    node->time = timeof(name, false);
    node->flags = (node->time? PROBABLE : 0);
    node->prereqs = nil;
    node->next = nil;

    if(DEBUG(D_TRACE)) 
        print("newnode(%s), time = %d\n", name, node->time);

    return node;
}
/*e: constructor newnode */

/*s: dumper dumpn */
void
dumpn(char *s, Node *n)
{
    char buf[1024];
    Arc *a;

    Bprint(&bout, "%s%s@%p: time=%ld flags=0x%x next=%p\n",
        s, n->name, n, n->time, n->flags, n->next);
    for(a = n->prereqs; a; a = a->next){
        snprint(buf, sizeof buf, "%s   ", (*s == ' ')? s:"");
        dumpa(buf, a);
    }
}
/*e: dumper dumpn */

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
            a = 0;
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
        if(a->n)
            ambiguous(a->n);
        if(*a->r->recipe == 0) continue;
        if(r == nil)
            r = a->r, la = a;
        else{
            if(r->recipe != a->r->recipe){
                if((r->attr&META) && !(a->r->attr&META)){
                    la->flag |= TOGO;
                    r = a->r, la = a;
                } else if(!(r->attr&META) && (a->r->attr&META)){
                    a->flag |= TOGO;
                    continue;
                }
            }
            if(r->recipe != a->r->recipe){
                if(bad == false){
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
    togo(n);
}
/*e: function ambiguous */

/*s: function attribute */
static void
attribute(Node *n)
{
    Arc *a;

    for(a = n->prereqs; a; a = a->next){
        if(a->r->attr&VIR)
            n->flags |= VIRTUAL;
        if(a->r->attr&NOREC)
            n->flags |= NORECIPE;
        if(a->r->attr&DEL)
            n->flags |= DELETE;
        if(a->n)
            attribute(a->n);
    }
    if(n->flags&VIRTUAL)
        n->time = 0;
}
/*e: function attribute */
/*e: mk/graph.c */
