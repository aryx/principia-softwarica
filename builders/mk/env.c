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


// was in plan9.c
/*s: function [[encodenulls]] */
/* break string of values into words at 01's or nulls*/
static Word *
encodenulls(char *s, int n)
{
    Word *head, *lastw;
    char *cp;

    head = lastw = nil;
    while (n-- > 0) {
        for (cp = s; *cp && *cp != '\0'; cp++)
                n--;
        *cp = '\0';

        // add_list(newword(s), head)
        if (lastw) {
            lastw->next = newword(s);
            lastw = lastw->next;
        } else
            head = lastw = newword(s);

        s = cp+1;
    }
    if (!head)
        head = newword("");
    return head;
}
/*e: function [[encodenulls]] */

// was in plan9.c
/*s: function [[readenv]] */
void
readenv(void)
{
    fdt envdir;
    fdt envfile;
    Dir *e;
    int i, n, len, len2;
    char *p;
    char name[1024];
    Word *w;

    rfork(RFENVG);	/*  use copy of the current environment variables */

    envdir = open("/env", OREAD);
    /*s: [[readenv()]] sanity check envdir */
    if(envdir < 0)
        return;
    /*e: [[readenv()]] sanity check envdir */
    while((n = dirread(envdir, &e)) > 0){
        for(i = 0; i < n; i++){
            len = e[i].length;
            /*s: [[readenv()]] skip some names */
            /* don't import funny names, NULL values,
             * or internal mk variables
             */
            if(len <= 0 
               || *shname(e[i].name) != '\0' 
               || symlook(e[i].name, S_INTERNAL, nil))
                continue;
            /*e: [[readenv()]] skip some names */

            snprint(name, sizeof name, "/env/%s", e[i].name);
            envfile = open(name, OREAD);
            /*s: [[readenv()]] sanity check envfile */
            if(envfile < 0)
                continue;
            /*e: [[readenv()]] sanity check envfile */
            p = Malloc(len+1);
            len2 = read(envfile, p, len);
            /*s: [[readenv()]] sanity check len2 */
            if(len2 != len){
                perror(name);
                close(envfile);
                continue;
            }
            /*e: [[readenv()]] sanity check len2 */
            close(envfile);
            /*s: [[readenv()]] add null terminator character at end of [[p]] */
            if (p[len-1] == '\0')
                len--;
            else
                p[len] = '\0';
            /*e: [[readenv()]] add null terminator character at end of [[p]] */
            w = encodenulls(p, len);
            free(p);
            p = strdup(e[i].name);

            // populating symbol table
            setvar(p, (void *) w);
        }
        free(e);
    }
    close(envdir);
}
/*e: function [[readenv]] */

// was in plan9.c
/*s: function [[exportenv]] */
/* as well as 01's, change blanks to nulls, so that rc will
 * treat the words as separate arguments
 */
void
exportenv(ShellEnvVar *e)
{
    int n;
    fdt f;
    bool first;
    Word *w;
    char name[256];
    /*s: [[exportenv()]] other locals */
    Symtab *sym;
    bool hasvalue;
    /*e: [[exportenv()]] other locals */

    for(;e->name; e++){
        /*s: [[exportenv()]] skip entry if not a user variable and no value */
        hasvalue = !empty_words(e->values);
        sym = symlook(e->name, S_VAR, nil);
        if(sym == nil && !hasvalue)	/* non-existant null symbol */
            continue;
        /*e: [[exportenv()]] skip entry if not a user variable and no value */
        // else
        snprint(name, sizeof name, "/env/%s", e->name);
        /*s: [[exportenv()]] if existing symbol but no value, remove from env */
        if (sym != nil && !hasvalue) {	/* Remove from environment */
            /* we could remove it from the symbol table
             * too, but we're in the child copy, and it
             * would still remain in the parent's table.
             */
            remove(name);
            freewords(e->values);
            e->values = nil;		/* memory leak */
            continue;
        }
        /*e: [[exportenv()]] if existing symbol but no value, remove from env */
        // else
        f = create(name, OWRITE, 0666L);
        /*s: [[exportenv()]] sanity check f */
        if(f < 0) {
            fprint(STDERR, "can't create %s, f=%d\n", name, f);
            perror(name);
            continue;
        }
        /*e: [[exportenv()]] sanity check f */
        first = true;
        for (w = e->values; w; w = w->next) {
            n = strlen(w->s);
            if (n) {
                /*s: [[exportenv()]] write null separator */
                if(first)
                    first = false;
                else{
                    if (write (f, "\000", 1) != 1)
                        perror(name);
                }
                /*e: [[exportenv()]] write null separator */
                if (write(f, w->s, n) != n)
                    perror(name);
            }
        }
        close(f);
    }
}
/*e: function [[exportenv]] */



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
