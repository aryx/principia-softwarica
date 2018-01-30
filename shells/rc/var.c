/*s: rc/var.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

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
