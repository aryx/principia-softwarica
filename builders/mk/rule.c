/*s: mk/rule.c */
#include	"mk.h"

/*s: global [[lr]] */
// option<ref<Rule>> (head = rules)
static Rule *lr;
/*e: global [[lr]] */
/*s: global [[lmr]] */
// option<ref<Rule>> (head = metarules)
static Rule *lmr;
/*e: global [[lmr]] */

/*s: global [[nrules]] */
static int nrules = 0;
/*e: global [[nrules]] */

static int rcmp(Rule *r, char *target, Word *tail);


/*s: function [[addrule]] */
void
addrule(char *target, Word *prereqs, char *recipe, 
        Word *alltargets, int attr, int hline, char *prog)
{
    Rule *r = nil;
    /*s: [[addrule()]] other locals */
    Symtab *sym;
    Rule *rr;
    /*x: [[addrule()]] other locals */
    bool reuse;
    /*e: [[addrule()]] other locals */

    /*s: [[addrule()]] find if rule already exists, set [[reuse]], update [[r]] */
    reuse = false;
    sym = symlook(target, S_TARGET, nil);
    if(sym){
        for(r = sym->u.ptr; r; r = r->chain)
            if(rcmp(r, target, prereqs) == 0){
                reuse = true;
                break;
            }
    }
    /*e: [[addrule()]] find if rule already exists, set [[reuse]], update [[r]] */

    if(r == nil)
        r = (Rule *)Malloc(sizeof(Rule));

    r->target = target;
    r->prereqs = prereqs;
    r->recipe = recipe;

    r->attr = attr;
    r->line = hline;
    /*s: [[addrule()]] set more fields */
    if(!reuse){
        r->next = nil;
    }
    /*x: [[addrule()]] set more fields */
    r->alltargets = alltargets;
    /*x: [[addrule()]] set more fields */
    r->file = infile;
    /*x: [[addrule()]] set more fields */
    r->rule = nrules++;
    /*x: [[addrule()]] set more fields */
    r->prog = prog;
    /*e: [[addrule()]] set more fields */

    /*s: [[addrule()]] indexing [[r]] by target in [[S_TARGET]] */
    if(!reuse){
        sym = symlook(target, S_TARGET, r);
        rr = sym->u.ptr;
        if(rr != r){ // target had already a rule
            r->chain = rr->chain;
            rr->chain = r;
        } else 
            r->chain = nil;
    }
    /*e: [[addrule()]] indexing [[r]] by target in [[S_TARGET]] */

    /*s: [[addrule()]] if meta rule */
    if(charin(target, "%&") || (attr&REGEXP)){
        r->attr |= META;
        /*s: [[addrule()]] return if reuse, to not add the rule in a list */
        if(reuse)
            return;
        /*e: [[addrule()]] return if reuse, to not add the rule in a list */
        // else
        /*s: [[addrule()]] if REGEXP attribute */
        if(attr&REGEXP){
            patrule = r;
            r->pat = regcomp(target);
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
        /*s: [[addrule()]] if simple rule */
        /*s: [[addrule()]] return if reuse, to not add the rule in a list */
        if(reuse)
            return;
        /*e: [[addrule()]] return if reuse, to not add the rule in a list */
        // else

        // add_list(r, rules, lr)
        if(rules == nil)
            rules = lr = r;
        else {
            lr->next = r;
            lr = r;
        }
        /*e: [[addrule()]] if simple rule */
    }
}
/*e: function [[addrule]] */


/*s: function [[rcmp]] */
static int
rcmp(Rule *r, char *target, Word *prereqs)
{
    Word *w;

    if(strcmp(r->target, target))
        return 1;
    for(w = r->prereqs; w && prereqs; w = w->next, prereqs = prereqs->next)
        if(strcmp(w->s, prereqs->s))
            return 1;
    return (w || prereqs);
}
/*e: function [[rcmp]] */

/*s: function [[rulecnt]] */
char*
rulecnt(void)
{
    char *s;

    s = Malloc(nrules);
    memset(s, 0, nrules);
    return s;
}
/*e: function [[rulecnt]] */

/*s: function [[regerror]] */
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
/*e: function [[regerror]] */
/*e: mk/rule.c */
