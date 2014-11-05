/*s: mk/env.c */
#include	"mk.h"

/*s: constant ENVQUANTA */
#define ENVQUANTA 10
/*e: constant ENVQUANTA */

/*s: global envy */
// array<ref_own<Envy>> (array or list?)
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
    "stem",
    "prereq",

    "pid",
    "nproc",
    "newprereq",
    "alltarget",
    "newmember",

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

    if(symlook(s->name, S_NOEXPORT, 0))
        return;
    for(p = myenv; *p; p++)
        if(strcmp(*p, s->name) == 0)
            return;
    envinsert(s->name, s->u.ptr);
}
/*e: function ecopy */

/*s: function execinit */
void
execinit(void)
{
    char **p;

    nextv = 0; // reset envy
    for(p = myenv; *p; p++)
        envinsert(*p, stow(""));

    symtraverse(S_VAR, ecopy);
    envinsert(nil, nil);
}
/*e: function execinit */

/*s: function buildenv */
Envy*
buildenv(Job *j, int slot)
{
    char **p, *cp, *qp;
    Word *w, *v, **l;
    int i;
    char buf[256];

    envupd("target", wdup(j->t));
    if(j->r->attr&REGEXP)
        envupd("stem",newword(""));
    else
        envupd("stem", newword(j->stem));
    envupd("prereq", wdup(j->p));
    snprint(buf, sizeof buf, "%d", getpid());
    envupd("pid", newword(buf));
    snprint(buf, sizeof buf, "%d", slot);
    envupd("nproc", newword(buf));
    envupd("newprereq", wdup(j->np));
    envupd("alltarget", wdup(j->at));
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
    return envy;
}
/*e: function buildenv */
/*e: mk/env.c */
