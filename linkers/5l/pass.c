/*s: linkers/5l/pass.c */
#include	"l.h"

/*s: function dodata(arm) */
void
dodata(void)
{
    int i, t;
    Sym *s;
    Prog *p;
    long orig;
    long v;

    DBG("%5.2f dodata\n", cputime());
    for(p = datap; p != P; p = p->link) {
        s = p->from.sym;
        /*s: [[dodata()]] if ADYNT or AINIT */
        if(p->as == ADYNT || p->as == AINIT)
            s->value = dtype;
        /*e: [[dodata()]] if ADYNT or AINIT */
        if(s->type == SBSS)
            s->type = SDATA;

        if(s->type != SDATA)
            diag("initialize non-data (%d): %s\n%P",
                s->type, s->name, p);
        v = p->from.offset + p->reg;
        if(v > s->value)
            diag("initialize bounds (%ld): %s\n%P",
                s->value, s->name, p);
    }
    /*s: [[dodata()]] if string in text segment */
    if(debug['t']) {
        /*
         * pull out string constants
         */
        for(p = datap; p != P; p = p->link) {
            s = p->from.sym;
            if(p->to.type == D_SCONST)
                s->type = SSTRING;
        }
    }
    /*e: [[dodata()]] if string in text segment */

    orig = 0;

    /*
     * pass 1
     *	assign 'small' variables to data segment
     *	(rational is that data segment is more easily
     *	 addressed through offset on R12)
     */
    for(i=0; i<NHASH; i++)
     for(s = hash[i]; s != S; s = s->link) {
        t = s->type;
        if(t != SDATA && t != SBSS)
            continue;

        v = s->value;
        if(v == 0) {
            diag("%s: no size", s->name);
            v = 1;
        }
        while(v & 3)
            v++;
        s->value = v;

        if(v > MINSIZ)
            continue;
        s->value = orig;
        orig += v;
        s->type = SDATA1;
    }

    /*
     * pass 2
     *	assign large 'data' variables to data segment
     */
    for(i=0; i<NHASH; i++)
     for(s = hash[i]; s != S; s = s->link) {
        t = s->type;
        if(t != SDATA) {
            if(t == SDATA1)
                s->type = SDATA;
            continue;
        }
        v = s->value;
        s->value = orig;
        orig += v;
    }

    while(orig & 7)
        orig++;

    datsize = orig;

    /*
     * pass 3
     *	everything else to bss segment
     */
    for(i=0; i<NHASH; i++)
     for(s = hash[i]; s != S; s = s->link) {
        if(s->type != SBSS)
            continue;
        v = s->value;
        s->value = orig;
        orig += v;
    }
    while(orig & 7)
        orig++;

    bsssize = orig-datsize;

    xdefine("setR12", SDATA, 0L+BIG);
    xdefine("bdata", SDATA, 0L);
    xdefine("edata", SDATA, datsize);
    xdefine("end", SBSS, datsize+bsssize);
    xdefine("etext", STEXT, 0L);
}
/*e: function dodata(arm) */

/*s: function undef */
void
undef(void)
{
    int i;
    Sym *s;

    for(i=0; i<NHASH; i++)
        for(s = hash[i]; s != S; s = s->link)
            if(s->type == SXREF)
                diag("%s: not defined", s->name);
}
/*e: function undef */

/*s: function brchain(arm) */
Prog*
brchain(Prog *p)
{
    int i;

    for(i=0; i<20; i++) {
        if(p == P || p->as != AB)
            return p;
        p = p->cond;
    }
    return P;
}
/*e: function brchain(arm) */

/*s: function relinv(arm) */
int
relinv(int a)
{
    switch(a) {
    case ABEQ:	return ABNE;
    case ABNE:	return ABEQ;
    case ABCS:	return ABCC;
    case ABHS:	return ABLO;
    case ABCC:	return ABCS;
    case ABLO:	return ABHS;
    case ABMI:	return ABPL;
    case ABPL:	return ABMI;
    case ABVS:	return ABVC;
    case ABVC:	return ABVS;
    case ABHI:	return ABLS;
    case ABLS:	return ABHI;
    case ABGE:	return ABLT;
    case ABLT:	return ABGE;
    case ABGT:	return ABLE;
    case ABLE:	return ABGT;
    }
    diag("unknown relation: %s", anames[a]);
    return a;
}
/*e: function relinv(arm) */

/*s: function follow */
void
follow(void)
{

    DBG("%5.2f follow\n", cputime());

    firstp = prg();
    lastp = firstp;

    xfol(textp);

    lastp->link = P;
    firstp = firstp->link;
}
/*e: function follow */

/*s: function xfol(arm) */
void
xfol(Prog *p)
{
    Prog *q, *r;
    int a, i;

loop:
    if(p == P)
        return;
    /*s: adjust curtext when iterate over instructions p */
    if(p->as == ATEXT)
        curtext = p;
    /*e: adjust curtext when iterate over instructions p */
    a = p->as;

    if(a == AB) {
        q = p->cond;
        if(q != P) {
            p->mark |= FOLL;
            p = q;
            if(!(p->mark & FOLL))
                goto loop;
        }
    }

    if(p->mark & FOLL) {
        /*s: [[xfol()]] when p is marked, for loop to copy instructions */
        for(i=0, q=p; i<4 && q != lastp; i++, q=q->link) {
            a = q->as;
            if(a == ANOP) {
                i--;
                continue;
            }
            if(a == AB || (a == ARET && q->scond == COND_ALWAYS) || a == ARFE)
                goto copy;
            if(!q->cond || (q->cond->mark & FOLL))
                continue;
            if(a != ABEQ && a != ABNE)
                continue;

        // here when a is one of AB, ARET, ARFE, ABEQ, ABNE
        copy:
            for(;;) {
                r = prg();
                *r = *p;

                /*s: [[xfol()]] sanity check one, r should be marked */
                if(!(r->mark&FOLL))
                    print("cant happen 1\n");
                r->mark |= FOLL;
                /*e: [[xfol()]] sanity check one, r should be marked */

                if(p != q) {
                    p = p->link;
                    lastp->link = r;
                    lastp = r;
                    continue;
                }
                lastp->link = r;
                lastp = r;

                if(a == AB || (a == ARET && q->scond == COND_ALWAYS) || a == ARFE)
                    return;

                // r->as = relinv(a)
                r->as = ABNE;
                if(a == ABNE)
                    r->as = ABEQ;

                r->cond = p->link;
                r->link = p->cond;

                if(!(r->link->mark&FOLL))
                    // recursive call
                    xfol(r->link);

                /*s: [[xfol()]] sanity check two, [[r->cond]] should be marked */
                if(!(r->cond->mark&FOLL))
                    print("cant happen 2\n");
                /*e: [[xfol()]] sanity check two, [[r->cond]] should be marked */

                return;
            }
        }
        /*e: [[xfol()]] when p is marked, for loop to copy instructions */
        a = AB;
        q = prg();
        q->as = a;
        q->line = p->line;
        q->to.type = D_BRANCH;
        q->to.offset = p->pc;
        q->cond = p;
        p = q;
    }

    p->mark |= FOLL;
    lastp->link = p;
    lastp = p;

    if(a == AB || (a == ARET && p->scond == COND_ALWAYS) || a == ARFE){
        return;
    }
    if(p->cond != P)
     /*s: [[xfol()]] if a is not ABL and p has a link */
     if(a != ABL && p->link != P) {
        q = brchain(p->link);

        if(a != ATEXT && a != ABCASE)
         if(q != P && (q->mark & FOLL)) {
            p->as = relinv(a);
            p->link = p->cond;
            p->cond = q;
        }

        // recursive call
        xfol(p->link);

        q = brchain(p->cond);
        if(q == P)
            q = p->cond;
        if(q->mark&FOLL) {
            p->cond = q;
            return;
        }
        p = q;
        goto loop;
     }
     /*e: [[xfol()]] if a is not ABL and p has a link */
    p = p->link;
    goto loop;
}
/*e: function xfol(arm) */

/*s: function patch(arm) */
void
patch(void)
{
    /*s: [[patch()]] locals */
    Prog *p, *q;
    int a;
    long c;
    Sym *s;
    /*x: [[patch()]] locals */
    long vexit;
    /*e: [[patch()]] locals */

    DBG("%5.2f patch\n", cputime());

    /*s: [[patch()]] initialisations */
    s = lookup("exit", 0);
    vexit = s->value;
    /*x: [[patch()]] initialisations */
    mkfwd();
    /*e: [[patch()]] initialisations */

    // pass 1
    for(p = firstp; p != P; p = p->link) {
        /*s: adjust curtext when iterate over instructions p */
        if(p->as == ATEXT)
            curtext = p;
        /*e: adjust curtext when iterate over instructions p */

        /*s: [[patch()]] resolve branch instructions using symbols */
        a = p->as;
        if((a == ABL || a == AB || a == ARET) &&
           p->to.type != D_BRANCH && 
           p->to.sym != S) {
            s = p->to.sym;
            switch(s->type) {
            case STEXT:
                p->to.offset = s->value;
                p->to.type = D_BRANCH;
                break;
            /*s: [[patch()]] switch section type for branch instruction, cases */
            default:
                diag("undefined: %s\n%P", s->name, p);
                s->type = STEXT;
                s->value = vexit;
                break;
            /*x: [[patch()]] switch section type for branch instruction, cases */
            case SUNDEF:
                if(p->as != ABL)
                    diag("help: SUNDEF in AB || ARET");
                p->to.offset = 0;
                p->to.type = D_BRANCH;
                p->cond = UP;
                break;
            /*e: [[patch()]] switch section type for branch instruction, cases */
            }
        }
        /*e: [[patch()]] resolve branch instructions using symbols */

        if(p->to.type != D_BRANCH || p->cond == UP)
            continue;

        c = p->to.offset;
        /*s: [[patch()]] find Prog q with pc == c */
        for(q = firstp; q != P;) {
            if(q->forwd != P)
             if(c >= q->forwd->pc) {
                q = q->forwd;
                continue;
            }
            if(c == q->pc)
                break;
            q = q->link;
        }
        if(q == P) {
            diag("branch out of range %ld\n%P", c, p);
            p->to.type = D_NONE;
        }
        /*e: [[patch()]] find Prog q with pc == c */
        p->cond = q;
    }

    // pass 2
    for(p = firstp; p != P; p = p->link) {
        /*s: adjust curtext when iterate over instructions p */
        if(p->as == ATEXT)
            curtext = p;
        /*e: adjust curtext when iterate over instructions p */
        if(p->cond != P && p->cond != UP) {
            p->cond = brloop(p->cond);
            if(p->cond != P)
             if(p->to.type == D_BRANCH)
                p->to.offset = p->cond->pc;
        }
    }
}
/*e: function patch(arm) */

/*s: constant LOG */
#define	LOG	5
/*e: constant LOG */
/*s: function mkfwd */
void
mkfwd(void)
{
    long dwn[LOG], cnt[LOG];
    Prog *lst[LOG];
    Prog *p;
    int i;

    for(i=0; i<LOG; i++) {
        if(i == 0)
            cnt[i] = 1; 
        else
            cnt[i] = LOG * cnt[i-1];
        dwn[i] = 1;
        lst[i] = P;
    }

    i = 0;
    for(p = firstp; p != P; p = p->link) {
        /*s: adjust curtext when iterate over instructions p */
        if(p->as == ATEXT)
            curtext = p;
        /*e: adjust curtext when iterate over instructions p */
        i--;
        if(i < 0)
            i = LOG-1;
        p->forwd = P;
        dwn[i]--;
        if(dwn[i] <= 0) {
            dwn[i] = cnt[i];
            if(lst[i] != P)
                lst[i]->forwd = p;
            lst[i] = p;
        }
    }
}
/*e: function mkfwd */

/*s: function brloop(arm) */
Prog*
brloop(Prog *p)
{
    Prog *q;
    int c = 0;

    for(; p!=P;) {
        if(p->as != AB)
            return p;
        q = p->cond;
        if(q <= p) {
            c++;
            if(q == p || c > 5000)
                break;
        }
        p = q;
    }
    return P;
}
/*e: function brloop(arm) */

/*s: function atolwhex */
long
atolwhex(char *s)
{
    long n;
    int f;

    n = 0;
    f = 0;
    while(*s == ' ' || *s == '\t')
        s++;
    if(*s == '-' || *s == '+') {
        if(*s++ == '-')
            f = 1;
        while(*s == ' ' || *s == '\t')
            s++;
    }
    if(s[0]=='0' && s[1]){
        if(s[1]=='x' || s[1]=='X'){
            s += 2;
            for(;;){
                if(*s >= '0' && *s <= '9')
                    n = n*16 + *s++ - '0';
                else if(*s >= 'a' && *s <= 'f')
                    n = n*16 + *s++ - 'a' + 10;
                else if(*s >= 'A' && *s <= 'F')
                    n = n*16 + *s++ - 'A' + 10;
                else
                    break;
            }
        } else
            while(*s >= '0' && *s <= '7')
                n = n*8 + *s++ - '0';
    } else
        while(*s >= '0' && *s <= '9')
            n = n*10 + *s++ - '0';
    if(f)
        n = -n;
    return n;
}
/*e: function atolwhex */

/*s: function rnd */
long
rnd(long v, long r)
{
    long c;

    if(r <= 0)
        return v;
    v += r - 1;
    c = v % r;
    if(c < 0)
        c += r;
    v -= c;
    return v;
}
/*e: function rnd */

/*s: function import(arm) */
void
import(void)
{
    int i;
    Sym *s;

    for(i = 0; i < NHASH; i++)
        for(s = hash[i]; s != S; s = s->link)
            if(s->sig != 0 && s->type == SXREF && (nimports == 0 || s->subtype == SIMPORT)){
                undefsym(s);
                Bprint(&bso, "IMPORT: %s sig=%lux v=%ld\n", s->name, s->sig, s->value);
            }
}
/*e: function import(arm) */

/*s: function ckoff */
void
ckoff(Sym *s, long v)
{
    if(v < 0 || v >= 1<<Roffset)
        diag("relocation offset %ld for %s out of range", v, s->name);
}
/*e: function ckoff */

/*s: function newdata(arm) */
static Prog*
newdata(Sym *s, int o, int w, int t)
{
    Prog *p;

    p = prg();
    p->link = datap;
    datap = p;

    p->as = ADATA;
    p->reg = w;
    p->from.type = D_OREG;
    p->from.symkind = t;
    p->from.sym = s;
    p->from.offset = o;
    p->to.type = D_CONST;
    p->to.symkind = D_NONE;

    return p;
}
/*e: function newdata(arm) */

/*s: function export(arm) */
void
export(void)
{
    int i, j, n, off, nb, sv, ne;
    Sym *s, *et, *str, **esyms;
    Prog *p;
    char buf[NSNAME], *t;

    n = 0;
    for(i = 0; i < NHASH; i++)
        for(s = hash[i]; s != S; s = s->link)
            if(s->sig != 0 && s->type != SXREF && s->type != SUNDEF && (nexports == 0 || s->subtype == SEXPORT))
                n++;
    esyms = malloc(n*sizeof(Sym*));
    ne = n;
    n = 0;
    for(i = 0; i < NHASH; i++)
        for(s = hash[i]; s != S; s = s->link)
            if(s->sig != 0 && s->type != SXREF && s->type != SUNDEF && (nexports == 0 || s->subtype == SEXPORT))
                esyms[n++] = s;
    for(i = 0; i < ne-1; i++)
        for(j = i+1; j < ne; j++)
            if(strcmp(esyms[i]->name, esyms[j]->name) > 0){
                s = esyms[i];
                esyms[i] = esyms[j];
                esyms[j] = s;
            }

    nb = 0;
    off = 0;
    et = lookup(EXPTAB, 0);
    if(et->type != 0 && et->type != SXREF)
        diag("%s already defined", EXPTAB);
    et->type = SDATA;
    str = lookup(".string", 0);
    if(str->type == 0)
        str->type = SDATA;
    sv = str->value;
    for(i = 0; i < ne; i++){
        s = esyms[i];
        Bprint(&bso, "EXPORT: %s sig=%lux t=%d\n", s->name, s->sig, s->type);

        /* signature */
        p = newdata(et, off, sizeof(long), D_EXTERN);
        off += sizeof(long);
        p->to.offset = s->sig;

        /* address */
        p = newdata(et, off, sizeof(long), D_EXTERN);
        off += sizeof(long);
        p->to.symkind = D_EXTERN;
        p->to.sym = s;

        /* string */
        t = s->name;
        n = strlen(t)+1;
        for(;;){
            buf[nb++] = *t;
            sv++;
            if(nb >= NSNAME){
                p = newdata(str, sv-NSNAME, NSNAME, D_STATIC);
                p->to.type = D_SCONST;
                p->to.sval = malloc(NSNAME);
                memmove(p->to.sval, buf, NSNAME);
                nb = 0;
            }
            if(*t++ == 0)
                break;
        }

        /* name */
        p = newdata(et, off, sizeof(long), D_EXTERN);
        off += sizeof(long);
        p->to.symkind = D_STATIC;
        p->to.sym = str;
        p->to.offset = sv-n;
    }

    if(nb > 0){
        p = newdata(str, sv-nb, nb, D_STATIC);
        p->to.type = D_SCONST;
        p->to.sval = malloc(NSNAME);
        memmove(p->to.sval, buf, nb);
    }

    for(i = 0; i < 3; i++){
        newdata(et, off, sizeof(long), D_EXTERN);
        off += sizeof(long);
    }
    et->value = off;
    if(sv == 0)
        sv = 1;
    str->value = sv;
    exports = ne;
    free(esyms);
}
/*e: function export(arm) */
/*e: linkers/5l/pass.c */
