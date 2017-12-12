/*s: mk/shprint.c */
#include	"mk.h"

static char *vexpand(char*, ShellEnvVar*, Bufblock*);
static char *shquote(char*, Rune, Bufblock*);
static char *shbquote(char*, Bufblock*);

/*s: function [[shprint]] */
void
shprint(char *s, ShellEnvVar *env, Bufblock *buf)
{
    Rune r;
    int n;

    while(*s) {
        n = chartorune(&r, s);
        if (r == '$')
            s = vexpand(s, env, buf);
        else {
            rinsert(buf, r);
            s += n;
            s = copyq(s, r, buf);	/*handle quoted strings*/
        }
    }
    insert(buf, 0);
}
/*e: function [[shprint]] */

/*s: function [[mygetenv]] */
static char*
mygetenv(char *name, ShellEnvVar *env)
{
    if (!env)
        return nil;
    if (!symlook(name, S_WESET, nil) && 
        !symlook(name, S_INTERNAL, nil))
        return nil;
    // else

    /* only resolve internal variables and variables we've set */
    for(; env->name; env++){
        if (strcmp(env->name, name) == 0)
            return wtos(env->values, ' ');
    }
    return nil;
}
/*e: function [[mygetenv]] */

/*s: function [[vexpand]] */
static char*
vexpand(char *w, ShellEnvVar *env, Bufblock *buf)
{
    char *s;
    char *p, *q;
    char carry;

    assert(/*vexpand no $*/ *w == '$');
    p = w+1;	/* skip dollar sign */
    if(*p == '{') {
        p++;
        q = utfrune(p, '}');
        if (!q)
            q = strchr(p, 0);
    } else
        q = shname(p);

    carry = *q;
    *q = '\0';
    s = mygetenv(p, env);
    *q = carry;

    if (carry == '}')
        q++;

    if (s) {
        bufcpy(buf, s, strlen(s));
        free(s);
    } else 		/* copy name intact*/
        bufcpy(buf, w, q-w);

    return q;
}
/*e: function [[vexpand]] */

/*s: function [[front]] */
void
front(char *s)
{
    char *t, *q;
    int i, j;
    char *flds[512];

    q = strdup(s);
    i = getfields(q, flds, nelem(flds), 0, " \t\n");
    if(i > 5){
        flds[4] = flds[i-1];
        flds[3] = "...";
        i = 5;
    }
    t = s;
    for(j = 0; j < i; j++){
        for(s = flds[j]; *s; *t++ = *s++);
        *t++ = ' ';
    }
    *t = 0;
    free(q);
}
/*e: function [[front]] */
/*e: mk/shprint.c */
