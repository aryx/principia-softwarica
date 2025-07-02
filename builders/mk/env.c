/*s: mk/env.c */
#include	"mk.h"

/*s: constant [[ENVQUANTA]] */
#define ENVQUANTA 10
/*e: constant [[ENVQUANTA]] */

/*s: global [[shellenv]] */
// growing_array<ShellEnvVar> (endmarker = (nil,nil), size = envinsert.envsize)
ShellEnvVar	*shellenv;
/*e: global [[shellenv]] */
/*s: global [[nextv]] */
// idx for next free entry in shellenv array
static int nextv;
/*e: global [[nextv]] */

/*s: global [[specialvars]] */
static char	*specialvars[] =
{
    "target",
    "prereq",
    "stem",

    /*s: [[specialvars]] other array elements */
    "stem0",		/* must be in order from here */
    "stem1",
    "stem2",
    "stem3",
    "stem4",
    "stem5",
    "stem6",
    "stem7",
    "stem8",
    "stem9",
    /*x: [[specialvars]] other array elements */
    "alltarget",
    /*x: [[specialvars]] other array elements */
    "newprereq",
    /*x: [[specialvars]] other array elements */
    "pid",
    /*x: [[specialvars]] other array elements */
    "nproc",
    /*x: [[specialvars]] other array elements */
    "newmember",
    /*e: [[specialvars]] other array elements */
    0,
};
/*e: global [[specialvars]] */

// encodenulls() is back in Plan9.c
// readenv() is back in Plan9.c
extern void readenv(void);
// exportenv() is back in plan9.c

/*s: function [[inithash]] */
void
inithash(void)
{
    char **p;

    for(p = specialvars; *p; p++)
        symlook(*p, S_INTERNAL, (void *)"");

    readenv();				/* o.s. dependent */
}
/*e: function [[inithash]] */

/*s: function [[envinsert]] */
static void
envinsert(char *name, Word *value)
{
    /*s: [[envinsert()]] locals */
    static int envsize = 0;
    /*e: [[envinsert()]] locals */

    /*s: [[envinsert()]] grow array if necessary */
    if (nextv >= envsize) {
        envsize += ENVQUANTA;
        shellenv = (ShellEnvVar *) Realloc((char *) shellenv, 
                                           envsize*sizeof(ShellEnvVar));
    }
    /*e: [[envinsert()]] grow array if necessary */
    shellenv[nextv].name = name;
    shellenv[nextv].values = value;
    nextv++;
}
/*e: function [[envinsert]] */

/*s: function [[envupd]] */
static void
envupd(char *name, Word *value)
{
    ShellEnvVar *e;

    for(e = shellenv; e->name; e++)
        if(strcmp(name, e->name) == 0){
            freewords(e->values);
            e->values = value;
            return;
        }
    /*s: [[envupd()]] if variable not found */
    // else
    e->name = name;
    e->values = value;
    envinsert(nil,nil);
    /*e: [[envupd()]] if variable not found */
}
/*e: function [[envupd]] */

/*s: function [[ecopy]] */
static void
ecopy(Symtab *s)
{
    char **p;

    /*s: [[ecopy()]] return and do not copy if [[S_NOEXPORT]] symbol */
    if(symlook(s->name, S_NOEXPORT, nil))
        return;
    /*e: [[ecopy()]] return and do not copy if [[S_NOEXPORT]] symbol */
    /*s: [[ecopy()]] return and do not copy if conflict with mk internal variable */
    for(p = specialvars; *p; p++)
        if(strcmp(*p, s->name) == 0)
            return;
    /*e: [[ecopy()]] return and do not copy if conflict with mk internal variable */
     // else
     envinsert(s->name, s->u.ptr);
}
/*e: function [[ecopy]] */

/*s: function [[initenv]] */
void
initenv(void)
{
    char **p;

    nextv = 0; // reset envy

    // internal mk variables
    for(p = specialvars; *p; p++)
        envinsert(*p, stow(""));

    // user variables in mkfiles, or mk process environment
    symtraverse(S_VAR, ecopy);

    // end marker
    envinsert(nil, nil);
}
/*e: function [[initenv]] */

/*s: function [[buildenv]] */
ShellEnvVar*
buildenv(Job *j, int slot)
{
    /*s: [[buildenv()]] locals */
    char **p;
    int i;
    /*x: [[buildenv()]] locals */
    char buf[256];
    /*x: [[buildenv()]] locals */
    Word *w, *v, **l;
    char *cp, *qp;
    /*e: [[buildenv()]] locals */

    // main variables 
    envupd("target", wdup(j->t));
    /*s: [[buildenv()]] if regexp rule */
    if(j->r->attr&REGEXP)
        envupd("stem", newword(""));
    /*e: [[buildenv()]] if regexp rule */
    else
        envupd("stem", newword(j->stem));
    envupd("prereq", wdup(j->p));

    // advanced variables 
    /*s: [[buildenv()]] envupd some variables */
    /* update stem0 -> stem9 */
    for(p = specialvars; *p; p++)
        if(strcmp(*p, "stem0") == 0)
            break;
    for(i = 0; *p; i++, p++){
        if((j->r->attr&REGEXP) && j->match[i])
            envupd(*p, newword(j->match[i]));
        else 
            envupd(*p, newword(""));
    }
    /*x: [[buildenv()]] envupd some variables */
    envupd("alltarget", wdup(j->at));
    /*x: [[buildenv()]] envupd some variables */
    envupd("newprereq", wdup(j->np));
    /*x: [[buildenv()]] envupd some variables */
    snprint(buf, sizeof buf, "%d", getpid());
    envupd("pid", newword(buf));
    /*x: [[buildenv()]] envupd some variables */
    snprint(buf, sizeof buf, "%d", slot);
    envupd("nproc", newword(buf));
    /*x: [[buildenv()]] envupd some variables */
    // newmember
    l = &v;
    v = w = wdup(j->np);
    while(w){
        cp = strchr(w->s, '(');
        if(cp){
            qp = strchr(cp+1, ')');
            if(qp){
                *qp = 0;
                strcpy(w->s, cp+1);
                l = &w->next;
                w = w->next;
                continue;
            }
        }
        *l = w->next;
        free(w->s);
        free(w);
        w = *l;
    }
    envupd("newmember", v);
    /*e: [[buildenv()]] envupd some variables */

    return shellenv;
}
/*e: function [[buildenv]] */
/*e: mk/env.c */
