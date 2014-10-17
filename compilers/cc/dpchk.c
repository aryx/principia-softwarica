/*s: cc/dpchk.c */
#include	"cc.h"
#include	"y.tab.h"

/*s: enum fxxx */
enum fxxx
{
    Fnone	= 0,

    Fl,
    Fvl,
    Fignor,
    Fstar,
    Fadj,

    Fverb	= 10,
};
/*e: enum fxxx */

typedef	struct	Tprot	Tprot;
/*s: struct Tprot */
struct	Tprot
{
    Type*	type;
    Bits	flag;
    Tprot*	link;
};
/*e: struct Tprot */

typedef	struct	Tname	Tname;
/*s: struct Tname */
struct	Tname
{
    char*	name;
    int	param;
    Tname*	link;
};
/*e: struct Tname */

/*s: global indchar */
static	Type*	indchar;
/*e: global indchar */
/*s: global flagbits */
static	uchar	flagbits[512];
/*e: global flagbits */
/*s: global fmtbuf */
static	char	fmtbuf[100];
/*e: global fmtbuf */
/*s: global lastadj */
static	int	lastadj;
/*e: global lastadj */
/*s: global lastverb */
static	int	lastverb;
/*e: global lastverb */
/*s: global nstar */
static	int	nstar;
/*e: global nstar */
/*s: global tprot */
static	Tprot*	tprot;
/*e: global tprot */
/*s: global tname */
static	Tname*	tname;
/*e: global tname */

/*s: function argflag */
void
argflag(int c, int v)
{

    switch(v) {
    case Fignor:
    case Fstar:
    case Fl:
    case Fvl:
        flagbits[c] = v;
        break;
    case Fverb:
        flagbits[c] = lastverb;
/*print("flag-v %c %d\n", c, lastadj);*/
        lastverb++;
        break;
    case Fadj:
        flagbits[c] = lastadj;
/*print("flag-l %c %d\n", c, lastadj);*/
        lastadj++;
        break;
    }
}
/*e: function argflag */

/*s: function getflag */
Bits
getflag(char *s)
{
    Bits flag;
    int f;
    char *fmt;
    Rune c;

    fmt = fmtbuf;
    flag = zbits;
    nstar = 0;
    for(;;) {
        s += chartorune(&c, s);
        fmt += runetochar(fmt, &c);
        if(c == 0 || c >= nelem(flagbits))
            break;
        f = flagbits[c];
        switch(f) {
        case Fnone:
            argflag(c, Fverb);
            f = flagbits[c];
            break;
        case Fstar:
            nstar++;
        case Fignor:
            continue;
        case Fl:
            if(bset(flag, Fl))
                flag = bor(flag, blsh(Fvl));
        }
        flag = bor(flag, blsh(f));
        if(f >= Fverb)
            break;
    }
    *fmt = 0;
    return flag;
}
/*e: function getflag */

/*s: function arginit */
void
arginit(void)
{
    int i;

/* debug['F'] = 1;*/
/* debug['w'] = 1;*/

    lastadj = Fadj;
    lastverb = Fverb;
    indchar = typ(TIND, types[TCHAR]);

    memset(flagbits, Fnone, sizeof(flagbits));

    for(i='0'; i<='9'; i++)
        argflag(i, Fignor);
    argflag('.', Fignor);
    argflag('#', Fignor);
    argflag('u', Fignor);
    argflag('h', Fignor);
    argflag('+', Fignor);
    argflag('-', Fignor);

    argflag('*', Fstar);
    argflag('l', Fl);

    argflag('o', Fverb);
    flagbits['x'] = flagbits['o'];
    flagbits['X'] = flagbits['o'];
}
/*e: function arginit */

/*s: function nextarg */
Node*
nextarg(Node *n, Node **a)
{
    if(n == Z) {
        *a = Z;
        return Z;
    }
    if(n->op == OLIST) {
        *a = n->left;
        return n->right;
    }
    *a = n;
    return Z;
}
/*e: function nextarg */

/*s: function checkargs */
void
checkargs(Node *nn, char *s, int pos)
{
    Node *a, *n;
    Bits flag;
    Tprot *l;

    if(!debug['F'])
        return;
    n = nn;
    for(;;) {
        s = strchr(s, '%');
        if(s == 0) {
            nextarg(n, &a);
            if(a != Z)
                warn(nn, "more arguments than format %T",
                    a->type);
            return;
        }
        s++;
        flag = getflag(s);
        while(nstar > 0) {
            n = nextarg(n, &a);
            pos++;
            nstar--;
            if(a == Z) {
                warn(nn, "more format than arguments %s",
                    fmtbuf);
                return;
            }
            if(a->type == T)
                continue;
            if(!sametype(types[TINT], a->type) &&
               !sametype(types[TUINT], a->type))
                warn(nn, "format mismatch '*' in %s %T, arg %d",
                    fmtbuf, a->type, pos);
        }
        for(l=tprot; l; l=l->link)
            if(sametype(types[TVOID], l->type)) {
                if(beq(flag, l->flag)) {
                    s++;
                    goto loop;
                }
            }

        n = nextarg(n, &a);
        pos++;
        if(a == Z) {
            warn(nn, "more format than arguments %s",
                fmtbuf);
            return;
        }
        if(a->type == 0)
            continue;
        for(l=tprot; l; l=l->link)
            if(sametype(a->type, l->type)) {
/*print("checking %T/%ulx %T/%ulx\n", a->type, flag.b[0], l->type, l->flag.b[0]);*/
                if(beq(flag, l->flag))
                    goto loop;
            }
        warn(nn, "format mismatch %s %T, arg %d", fmtbuf, a->type, pos);
    loop:;
    }
}
/*e: function checkargs */

/*s: function dpcheck */
void
dpcheck(Node *n)
{
    char *s;
    Node *a, *b;
    Tname *l;
    int i;

    if(n == Z)
        return;
    b = n->left;
    if(b == Z || b->op != ONAME)
        return;
    s = b->sym->name;
    for(l=tname; l; l=l->link)
        if(strcmp(s, l->name) == 0)
            break;
    if(l == 0)
        return;

    i = l->param;
    b = n->right;
    while(i > 0) {
        b = nextarg(b, &a);
        i--;
    }
    if(a == Z) {
        warn(n, "cant find format arg");
        return;
    }
    if(!sametype(indchar, a->type)) {
        warn(n, "format arg type %T", a->type);
        return;
    }
    if(a->op != OADDR || a->left->op != ONAME || a->left->sym != symstring) {
/*		warn(n, "format arg not constant string");*/
        return;
    }
    s = a->left->cstring;
    checkargs(b, s, l->param);
}
/*e: function dpcheck */
/*e: cc/dpchk.c */
