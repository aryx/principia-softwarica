/*s: mk/rule.c */
#include	"mk.h"

/*s: global lr */
// option<ref<Rule>> (head = rules)
static Rule *lr;
/*e: global lr */
/*s: global lmr */
// option<ref<Rule>> (head = metarules)
static Rule *lmr;
/*e: global lmr */

/*s: global nrules */
static int nrules = 0;
/*e: global nrules */

static int rcmp(Rule *r, char *target, Word *tail);


/*s: function addrule */
void
addrule(char *head, Word *tail, char *body, Word *ahead, int attr, int hline, char *prog)
{
    Rule *r = nil;
    /*s: [[addrule()]] other locals */
    Symtab *sym;
    bool reuse;
    /*x: [[addrule()]] other locals */
    Rule *rr;
    /*e: [[addrule()]] other locals */

    /*s: [[addrule()]] find if rule already exists */
    reuse = false;
    if(sym = symlook(head, S_TARGET, nil)){
        for(r = sym->u.ptr; r; r = r->chain)
            if(rcmp(r, head, tail) == 0){
                reuse = true;
                break;
            }
    }
    /*e: [[addrule()]] find if rule already exists */

    if(r == nil)
        r = (Rule *)Malloc(sizeof(Rule));

    r->target = head;
    r->tail = tail;

    r->recipe = body;

    r->line = hline;
    r->file = infile;

    r->attr = attr;

    /*s: [[addrule()]] set more fields */
    r->alltargets = ahead;
    /*x: [[addrule()]] set more fields */
    r->rule = nrules++;
    /*x: [[addrule()]] set more fields */
    r->prog = prog;
    /*e: [[addrule()]] set more fields */

    /*s: [[addrule()]] indexing [[r]] by target in [[S_TARGET]] */
    if(!reuse){
        sym = symlook(head, S_TARGET, r);
        rr = sym->u.ptr;
        if(rr != r){
            r->chain = rr->chain;
            rr->chain = r;
        } else 
            r->chain = nil;
        r->next = nil;
    }
    /*e: [[addrule()]] indexing [[r]] by target in [[S_TARGET]] */

    /*s: [[addrule()]] if meta rule */
    if(charin(head, "%&") || (attr&REGEXP)){
        r->attr |= META;
        if(reuse)
            return;
        /*s: [[addrule()]] if REGEXP attribute */
        if(attr&REGEXP){
            patrule = r;
            r->pat = regcomp(head);
        }
        /*e: [[addrule()]] if REGEXP attribute */

        // add_list(r, metarules, lmr)
        if(metarules == nil)
            metarules = lmr = r;
        else {
            lmr->next = r;
            lmr = r;
        }
    }
    /*e: [[addrule()]] if meta rule */
    else {
        if(reuse)
            return;
        r->pat = nil;

        // add_list(r, rules, lr)
        if(rules == nil)
            rules = lr = r;
        else {
            lr->next = r;
            lr = r;
        }
    }
}
/*e: function addrule */

/*s: dumper dumpr */
void
dumpr(char *s, Rule *r)
{
    Bprint(&bout, "%s: start=%p\n", s, r);
    for(; r; r = r->next){
        Bprint(&bout, "\tRule %p: %s:%d attr=%x next=%p chain=%p alltarget='%s'",
            r, r->file, r->line, r->attr, r->next, r->chain, wtos(r->alltargets, ' '));
        if(r->prog)
            Bprint(&bout, " prog='%s'", r->prog);
        Bprint(&bout, "\n\ttarget=%s: %s\n", r->target, wtos(r->tail,' '));
        Bprint(&bout, "\trecipe@%p='%s'\n", r->recipe, r->recipe);
    }
}
/*e: dumper dumpr */

/*s: function rcmp */
static int
rcmp(Rule *r, char *target, Word *tail)
{
    Word *w;

    if(strcmp(r->target, target))
        return 1;
    for(w = r->tail; w && tail; w = w->next, tail = tail->next)
        if(strcmp(w->s, tail->s))
            return 1;
    return (w || tail);
}
/*e: function rcmp */

/*s: function rulecnt */
char*
rulecnt(void)
{
    char *s;

    s = Malloc(nrules);
    memset(s, 0, nrules);
    return s;
}
/*e: function rulecnt */

/*s: function regerror */
//@Scheck: not dead, called via regcomp() when have regexp syntax error
void regerror(char *s)
{
    if(patrule)
        fprint(STDERR, "mk: %s:%d: regular expression error; %s\n",
            patrule->file, patrule->line, s);
    else
        fprint(STDERR, "mk: %s:%d: regular expression error; %s\n",
            infile, mkinline, s);
    Exit();
}
/*e: function regerror */
/*e: mk/rule.c */
