/*s: mk/env.c */
#include	"mk.h"

/*s: constant ENVQUANTA */
#define ENVQUANTA 10
/*e: constant ENVQUANTA */

/*s: global envy */
// growing_array<Envy> (endmarker = nil,nil)
Envy	*envy;
/*e: global envy */
/*s: global nextv */
// idx for next free entry in envy array
static int nextv;
/*e: global nextv */

/*s: global myenv */
static char	*myenv[] =
{
    "target",
    "prereq",
    "stem",

    /*s: [[myenv]] other array elements */
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
    /*x: [[myenv]] other array elements */
    "alltarget",
    /*x: [[myenv]] other array elements */
    "newprereq",
    /*x: [[myenv]] other array elements */
    "pid",
    /*x: [[myenv]] other array elements */
    "nproc",
    /*x: [[myenv]] other array elements */
    "newmember",
    /*e: [[myenv]] other array elements */
    0,
};
/*e: global myenv */

/*s: function initenv */
void
initenv(void)
{
    char **p;

    for(p = myenv; *p; p++)
        symlook(*p, S_INTERNAL, (void *)"");

    readenv();				/* o.s. dependent */
}
/*e: function initenv */

/*s: function envinsert */
static void
envinsert(char *name, Word *value)
{
    static int envsize;

    // grow array if necessary
    if (nextv >= envsize) {
        envsize += ENVQUANTA;
        envy = (Envy *) Realloc((char *) envy, envsize*sizeof(Envy));
    }

    envy[nextv].name = name;
    envy[nextv++].values = value;
}
/*e: function envinsert */

/*s: function envupd */
static void
envupd(char *name, Word *value)
{
    Envy *e;

    for(e = envy; e->name; e++)
        if(strcmp(name, e->name) == 0){
            delword(e->values);
            e->values = value;
            return;
        }
    e->name = name;
    e->values = value;
    envinsert(nil,nil); // ???
}
/*e: function envupd */

/*s: function ecopy */
static void
ecopy(Symtab *s)
{
    char **p;

    /*s: [[ecopy()]] return and do not copy if S_NOEXPORT symbol */
    if(symlook(s->name, S_NOEXPORT, nil))
        return;
    /*e: [[ecopy()]] return and do not copy if S_NOEXPORT symbol */
    /*s: [[ecopy()]] return and do not copy if conflict with mk internal variable */
    for(p = myenv; *p; p++)
        if(strcmp(*p, s->name) == 0)
            return;
    /*e: [[ecopy()]] return and do not copy if conflict with mk internal variable */
     // else
     envinsert(s->name, s->u.ptr);
}
/*e: function ecopy */

/*s: function execinit */
void
execinit(void)
{
    char **p;

    nextv = 0; // reset envy

    // internal mk variables
    for(p = myenv; *p; p++)
        envinsert(*p, stow(""));

    // user variables in mkfile or process shell environment
    symtraverse(S_VAR, ecopy);

    // end marker
    envinsert(nil, nil);
}
/*e: function execinit */

/*s: function buildenv */
Envy*
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
    for(p = myenv; *p; p++)
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

    return envy;
}
/*e: function buildenv */
/*e: mk/env.c */
