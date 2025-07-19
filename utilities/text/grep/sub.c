/*s: grep/sub.c */
#include    "grep.h"

/*s: function [[mal]](grep) */
void*
mal(int n)
{
    static char *s;
    static int m = 0;
    void *v;

    n = (n+3) & ~3;
    if(m < n) {
        if(n > Nhunk) {
            v = sbrk(n);
            memset(v, 0, n);
            return v;
        }
        s = sbrk(Nhunk);
        m = Nhunk;
    }
    v = s;
    s += n;
    m -= n;
    memset(v, 0, n);
    return v;
}
/*e: function [[mal]](grep) */
/*s: function [[sal]](grep) */
State*
sal(int n)
{
    State *s;

    s = mal(sizeof(*s));
//  s->next = mal(256*sizeof(*s->next));
    s->count = n;
    s->re = mal(n*sizeof(*state0->re));
    return s;
}
/*e: function [[sal]](grep) */
/*s: function [[ral]](grep) */
Re*
ral(int type)
{
    Re *r;

    r = mal(sizeof(*r));
    r->type = type;
    maxfollow++;
    return r;
}
/*e: function [[ral]](grep) */

/*s: function [[error]](grep) */
void
error(char *s)
{
    fprint(STDERR, "grep: internal error: %s\n", s);
    exits(s);
}
/*e: function [[error]](grep) */

// addcase() helpers
/*s: function [[countor]](grep) */
int
countor(Re *r)
{
    int n;

    n = 0;
loop:
    switch(r->type) {
    case Tor:
        n += countor(r->alt);
        r = r->next;
        goto loop;
    case Tclass:
        return n + r->hi - r->lo + 1;
    }
    return n;
}
/*e: function [[countor]](grep) */
/*s: function [[oralloc]](grep) */
Re*
oralloc(int t, Re *r, Re *b)
{
    Re *a;

    if(b == 0)
        return r;
    a = ral(t);
    a->alt = r;
    a->next = b;
    return a;
}
/*e: function [[oralloc]](grep) */
/*s: function [[case1]](grep) */
void
case1(Re *c, Re *r)
{
    int n;

loop:
    switch(r->type) {
    case Tor:
        case1(c, r->alt);
        r = r->next;
        goto loop;

    case Tclass:    /* add to character */
        for(n=r->lo; n<=r->hi; n++)
            c->cases[n] = oralloc(Tor, r->next, c->cases[n]);
        break;

    default:    /* add everything unknown to next */
        c->next = oralloc(Talt, r, c->next);
        break;
    }
}
/*e: function [[case1]](grep) */

/*s: function [[addcase]](grep) */
Re*
addcase(Re *r)
{
    int i, n;
    Re *a;

    if(r->gen == gen)
        return r;
    r->gen = gen;
    switch(r->type) {
    default:
        error("addcase");

    case Tor:
        n = countor(r);
        if(n >= Caselim) {
            a = ral(Tcase);
            a->cases = mal(256*sizeof(*a->cases));
            case1(a, r);
            for(i=0; i<256; i++)
                if(a->cases[i]) {
                    r = a->cases[i];
                    if(countor(r) < n)
                        a->cases[i] = addcase(r);
                }
            return a;
        }
        return r;

    case Talt:
        r->next = addcase(r->next);
        r->alt = addcase(r->alt);
        return r;

    case Tbegin:
    case Tend:
    case Tclass:
        return r;
    }
}
/*e: function [[addcase]](grep) */

/*s: function [[str2top]](grep) */
void
str2top(char *p)
{
    Re2 oldtop;

    oldtop = topre;
    input = p;
    if (*p == '\0')
        yyerror("empty pattern");   /* can't be a file name here */
    if (!flags['f'])
        pattern = p;

    topre.beg = nil;
    topre.end = nil;

    yyparse();

    gen++;
    if(topre.beg == nil)
        yyerror("syntax");
    if(oldtop.beg)
        topre = re2or(oldtop, topre);
}
/*e: function [[str2top]](grep) */

/*s: function [[appendnext]](grep) */
void
appendnext(Re *a, Re *b)
{
    Re *n;

    while(n = a->next)
        a = n;
    a->next = b;
}
/*e: function [[appendnext]](grep) */
/*s: function [[patchnext]](grep) */
void
patchnext(Re *a, Re *b)
{
    Re *n;

    while(a) {
        n = a->next;
        a->next = b;
        a = n;
    }
}
/*e: function [[patchnext]](grep) */

/*s: function [[getrec]](grep) */
int
getrec(void)
{
    int c;

    if(flags['f']) {
        c = Bgetc(rein);
        if(c <= 0)
            return 0;
    } else
        c = *input++ & 0xff;
    if(flags['i'] && c >= 'A' && c <= 'Z')
        c += 'a'-'A';
    if(c == '\n')
        lineno++;
    return c;
}
/*e: function [[getrec]](grep) */

/*s: function [[re2cat]](grep) */
Re2
re2cat(Re2 a, Re2 b)
{
    Re2 c;

    c.beg = a.beg;
    c.end = b.end;
    patchnext(a.end, b.beg);
    return c;
}
/*e: function [[re2cat]](grep) */
/*s: function [[re2star]](grep) */
Re2
re2star(Re2 a)
{
    Re2 c;

    c.beg = ral(Talt);
    c.beg->alt = a.beg;
    patchnext(a.end, c.beg);
    c.end = c.beg;
    return c;
}
/*e: function [[re2star]](grep) */
/*s: function [[re2or]](grep) */
Re2
re2or(Re2 a, Re2 b)
{
    Re2 c;

    c.beg = ral(Tor);
    c.beg->alt = b.beg;
    c.beg->next = a.beg;
    c.end = b.end;
    appendnext(c.end,  a.end);
    return c;
}
/*e: function [[re2or]](grep) */
/*s: function [[re2char]](grep) */
Re2
re2char(int c0, int c1)
{
    Re2 c;

    c.beg = ral(Tclass);
    c.beg->lo = c0 & 0xff;
    c.beg->hi = c1 & 0xff;
    c.end = c.beg;
    return c;
}
/*e: function [[re2char]](grep) */

/*s: function [[reprint1]](grep) */
void
reprint1(Re *a)
{
    int i, j;

loop:
    if(a == 0)
        return;
    if(a->gen == gen)
        return;
    a->gen = gen;
    print("%p: ", a);
    switch(a->type) {
    default:
        print("type %d\n", a->type);
        error("print1 type");

    case Tcase:
        print("case ->%p\n", a->next);
        for(i=0; i<256; i++)
            if(a->cases[i]) {
                for(j=i+1; j<256; j++)
                    if(a->cases[i] != a->cases[j])
                        break;
                print(" [%.2x-%.2x] ->%p\n", i, j-1, a->cases[i]);
                i = j-1;
            }
        for(i=0; i<256; i++)
            reprint1(a->cases[i]);
        break;

    case Tbegin:
        print("^ ->%p\n", a->next);
        break;

    case Tend:
        print("$ ->%p\n", a->next);
        break;

    case Tclass:
        print("[%.2x-%.2x] ->%p\n", a->lo, a->hi, a->next);
        break;

    case Tor:
    case Talt:
        print("| %p ->%p\n", a->alt, a->next);
        reprint1(a->alt);
        break;
    }
    a = a->next;
    goto loop;
}
/*e: function [[reprint1]](grep) */
/*s: function [[reprint]](grep) */
void
reprint(char *s, Re *r)
{
    print("%s:\n", s);
    gen++;
    reprint1(r);
    print("\n\n");
}
/*e: function [[reprint]](grep) */
/*e: grep/sub.c */
