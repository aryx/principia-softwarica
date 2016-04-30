/*s: linkers/5l/dynamic.c */
#include	"l.h"

// forward decls
typedef struct Reloc Reloc;

/*s: enum _anon_ (linkers/5l/span.c)(arm) */
enum{
    ABSD = 0,
    ABSU = 1,
    RELD = 2,
    RELU = 3,
};
/*e: enum _anon_ (linkers/5l/span.c)(arm) */

/*s: global modemap */
int modemap[4] = { 0, 1, -1, 2, };
/*e: global modemap */

/*s: struct Reloc */
struct Reloc
{
    int n;
    int t;
    byte *m;
    ulong *a;
};
/*e: struct Reloc */

/*s: global rels */
Reloc rels;
/*e: global rels */


/*s: global imports */
int	imports;
/*e: global imports */
/*s: global nimports */
int nimports;
/*e: global nimports */
/*s: global exports */
int	exports;
/*e: global exports */
/*s: global nexports */
int nexports;
/*e: global nexports */



/*s: function zerosig */
void
zerosig(char *sp)
{
    Sym *s;

    s = lookup(sp, 0);
    s->sig = 0;
}
/*e: function zerosig */

/*s: function readundefs */
void
readundefs(char *f, int t)
{
    int i, n;
    Sym *s;
    Biobuf *b;
    char *l, buf[256], *fields[64];

    if(f == nil)
        return;
    b = Bopen(f, OREAD);
    if(b == nil){
        diag("could not open %s: %r", f);
        errorexit();
    }
    while((l = Brdline(b, '\n')) != nil){
        n = Blinelen(b);
        if(n >= sizeof(buf)){
            diag("%s: line too long", f);
            errorexit();
        }
        memmove(buf, l, n);
        buf[n-1] = '\0';
        n = getfields(buf, fields, nelem(fields), 1, " \t\r\n");
        if(n == nelem(fields)){
            diag("%s: bad format", f);
            errorexit();
        }
        for(i = 0; i < n; i++){
            s = lookup(fields[i], 0);
            s->type = SXREF;
            s->subtype = t;
            if(t == SIMPORT)
                nimports++;
            else
                nexports++;
        }
    }
    Bterm(b);
}
/*e: function readundefs */

/*s: function undefsym */
void
undefsym(Sym *s)
{
    int n;

    n = imports;
    if(s->value != 0)
        diag("value != 0 on SXREF");
    if(n >= 1<<Rindex)
        diag("import index %d out of range", n);
    s->value = n<<Roffset;
    s->type = SUNDEF;
    imports++;
}
/*e: function undefsym */

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
    p->to.symkind = N_NONE;

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
        p = newdata(et, off, sizeof(long), N_EXTERN);
        off += sizeof(long);
        p->to.offset = s->sig;

        /* address */
        p = newdata(et, off, sizeof(long), N_EXTERN);
        off += sizeof(long);
        p->to.symkind = N_EXTERN;
        p->to.sym = s;

        /* string */
        t = s->name;
        n = strlen(t)+1;
        for(;;){
            buf[nb++] = *t;
            sv++;
            if(nb >= NSNAME){
                p = newdata(str, sv-NSNAME, NSNAME, N_INTERN);
                p->to.type = D_SCONST;
                p->to.sval = malloc(NSNAME);
                memmove(p->to.sval, buf, NSNAME);
                nb = 0;
            }
            if(*t++ == 0)
                break;
        }

        /* name */
        p = newdata(et, off, sizeof(long), N_EXTERN);
        off += sizeof(long);
        p->to.symkind = N_INTERN;
        p->to.sym = str;
        p->to.offset = sv-n;
    }

    if(nb > 0){
        p = newdata(str, sv-nb, nb, N_INTERN);
        p->to.type = D_SCONST;
        p->to.sval = malloc(NSNAME);
        memmove(p->to.sval, buf, nb);
    }

    for(i = 0; i < 3; i++){
        newdata(et, off, sizeof(long), N_EXTERN);
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



/*s: function grow */
static void
grow(Reloc *r)
{
    int t;
    byte *m, *nm;
    ulong *a, *na;

    t = r->t;
    r->t += 64;
    m = r->m;
    a = r->a;
    r->m = nm = malloc(r->t * sizeof(byte));
    r->a = na = malloc(r->t * sizeof(ulong));
    memmove(nm, m, t*sizeof(byte));
    memmove(na, a, t*sizeof(ulong));
    free(m);
    free(a);
}
/*e: function grow */

/*s: function dynreloc(arm) */
void
dynreloc(Sym *s, long v, int abs)
{
    int i, k, n;
    byte *m;
    ulong *a;
    Reloc *r;

    if(v&3)
        diag("bad relocation address");
    v >>= 2;

    if(s != S && s->type == SUNDEF)
        k = abs ? ABSU : RELU;
    else
        k = abs ? ABSD : RELD;
    /* Bprint(&bso, "R %s a=%ld(%lx) %d\n", s->name, a, a, k); */
    k = modemap[k];
    r = &rels;
    n = r->n;
    if(n >= r->t)
        grow(r);
    m = r->m;
    a = r->a;
    for(i = n; i > 0; i--){
        if(v < a[i-1]){	/* happens occasionally for data */
            m[i] = m[i-1];
            a[i] = a[i-1];
        }
        else
            break;
    }
    m[i] = k;
    a[i] = v;
    r->n++;
}
/*e: function dynreloc(arm) */

/*s: function sput */
static int
sput(char *s)
{
    char *p;

    p = s;
    while(*s)
        cput(*s++);
    cput(0);
    return  s-p+1;
}
/*e: function sput */

/*s: function asmdyn */
void
asmdyn()
{
    int i, n, t, c;
    Sym *s;
    ulong la, ra, *a;
    vlong off;
    byte *m;
    Reloc *r;

    cflush();
    off = seek(cout, 0, 1);
    lput(0);
    t = 0;
    lput(imports);
    t += 4;
    for(i = 0; i < NHASH; i++)
        for(s = hash[i]; s != S; s = s->link)
            if(s->type == SUNDEF){
                lput(s->sig);
                t += 4;
                t += sput(s->name);
            }

    la = 0;
    r = &rels;
    n = r->n;
    m = r->m;
    a = r->a;
    lput(n);
    t += 4;
    for(i = 0; i < n; i++){
        ra = *a-la;
        if(*a < la)
            diag("bad relocation order");
        if(ra < 256)
            c = 0;
        else if(ra < 65536)
            c = 1;
        else
            c = 2;
        cput((c<<6)|*m++);
        t++;
        if(c == 0){
            cput(ra);
            t++;
        }
        else if(c == 1){
            wput(ra);
            t += 2;
        }
        else{
            lput(ra);
            t += 4;
        }
        la = *a++;
    }

    cflush();
    seek(cout, off, 0);
    lput(t);

    DBG("import table entries = %d\n", imports);
    DBG("export table entries = %d\n", exports);
}
/*e: function asmdyn */

/*e: linkers/5l/dynamic.c */
