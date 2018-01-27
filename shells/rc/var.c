/*s: rc/var.c */
/*s: includes */
#include "rc.h"
#include "getflags.h"
#include "exec.h"
#include "io.h"
#include "fns.h"
/*e: includes */
#include "x.tab.h"

// was in rc.h
/*s: global [[gvar]] */
// map<string, ref_own<Var>> (next = Var.next in bucket list)
var *gvar[NVAR];		/* hash for globals */
/*e: global [[gvar]] */

/*s: function [[hash]] */
unsigned
hash(char *as, int n)
{
    int i = 1;
    unsigned h = 0;
    uchar *s;

    s = (uchar *)as;
    while (*s)
        h += *s++ * i++;
    return h % n;
}
/*e: function [[hash]] */


/*s: constant [[NKW]] */
#define	NKW	30
/*e: constant [[NKW]] */
/*s: struct [[Kw]] */
struct Kw {
    char *name;
    int type;

    struct Kw *next;
};
/*e: struct [[Kw]] */
/*s: global [[kw]] */
struct Kw *kw[NKW];
/*e: global [[kw]] */

/*s: function [[kenter]] */
void
kenter(int type, char *name)
{
    int h = hash(name, NKW);
    struct Kw *p = new(struct Kw);
    p->type = type;
    p->name = name;
    p->next = kw[h];
    kw[h] = p;
}
/*e: function [[kenter]] */

/*s: function [[kinit]] */
void
kinit(void)
{
    kenter(FOR, "for");
    kenter(IN, "in");
    kenter(WHILE, "while");
    kenter(IF, "if");
    kenter(NOT, "not");
    kenter(SWITCH, "switch");
    kenter(FN, "fn");

    kenter(TWIDDLE, "~");
    kenter(BANG, "!");
    kenter(SUBSHELL, "@");
}
/*e: function [[kinit]] */

/*s: function [[klook]] */
tree*
klook(char *name)
{
    struct Kw *p;
    tree *t = token(name, WORD);

    for(p = kw[hash(name, NKW)];p;p = p->next)
        if(strcmp(p->name, name)==0){
            t->type = p->type;
            break;
        }
    return t;
}
/*e: function [[klook]] */



/*s: function [[gvlook]] */
var*
gvlook(char *name)
{
    int h = hash(name, NVAR);
    var *v;

    for(v = gvar[h];v;v = v->next) 
        if(strcmp(v->name, name)==0) 
            return v;
    gvar[h] = newvar(strdup(name), gvar[h]);
    return gvar[h];
}
/*e: function [[gvlook]] */

/*s: function [[vlook]] */
var*
vlook(char *name)
{
    var *v;
    if(runq)
        for(v = runq->local;v;v = v->next)
            if(strcmp(v->name, name)==0) 
                return v;

    return gvlook(name);
}
/*e: function [[vlook]] */

/*s: function [[setvar]] */
void
setvar(char *name, word *val)
{
    struct Var *v = vlook(name);
    freewords(v->val);
    v->val = val;
    v->changed = true;
}
/*e: function [[setvar]] */

/*s: function [[newvar]] */
var*
newvar(char *name, var *next)
{
    var *v = new(var);
    v->name = name;
    v->val = nil;

    v->fn = nil;
    v->changed = false;
    v->fnchanged = false;

    v->next = next;
    return v;
}
/*e: function [[newvar]] */

/*e: rc/var.c */
