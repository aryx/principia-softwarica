/*s: linkers/5l/span.c */
#include	"l.h"

/*s: global pool(arm) */
static struct {
    ulong	start;
    ulong	size;
} pool;
/*e: global pool(arm) */

void	checkpool(Prog*);
void 	flushpool(Prog*, int);
typedef struct Reloc Reloc;


/*s: function span(arm) */
void
span(void)
{
    /*s: [[span()]] locals */
    Prog *p;
    Optab *o;
    long c, otxt;
    long v;
    Sym *setext, *s;
    int m, i;
    /*x: [[span()]] locals */
    bool bflag;
    /*e: [[span()]] locals */

    DBG("%5.2f span\n", cputime());

    /*s: [[span()]] initialisation */
    bflag = false;
    /*e: [[span()]] initialisation */

    c = INITTEXT;
    otxt = c;
    for(p = firstp; p != P; p = p->link) {
       /*s: adjust curtext when iterate over instructions p */
       if(p->as == ATEXT)
           curtext = p;
       /*e: adjust curtext when iterate over instructions p */

        p->pc = c;
        o = oplook(p);
        m = o->size;

        if(m == 0) {
            if(p->as == ATEXT) {
                autosize = p->to.offset + 4;
                if(p->from.sym != S)
                    p->from.sym->value = c;
                /*s: [[span()]] if large procedure */
                /* need passes to resolve branches */
                if(c - otxt >= 1L<<17)
                    bflag = true;
                /*x: [[span()]] if large procedure */
                /*
                 * if any procedure is large enough to
                 * generate a large SBRA branch, then
                 * generate extra passes putting branches
                 * around jmps to fix. this is rare.
                 */
                while(bflag) {
                    DBG("%5.2f span1\n", cputime());
                    bflag = false;
                    c = INITTEXT;
                    for(p = firstp; p != P; p = p->link) {
                        /*s: adjust curtext when iterate over instructions p */
                        if(p->as == ATEXT)
                            curtext = p;
                        /*e: adjust curtext when iterate over instructions p */
                        p->pc = c;
                        o = oplook(p);
                        m = o->size;
                        if(m == 0) {
                            if(p->as == ATEXT) {
                                autosize = p->to.offset + 4;
                                if(p->from.sym != S)
                                    p->from.sym->value = c;
                            } else {
                                diag("zero-width instruction\n%P", p);
                            }
                            continue;
                        }
                        c += m;
                    }
                }
                /*e: [[span()]] if large procedure */
                otxt = c;
            } else {
                diag("zero-width instruction\n%P", p);
            }
            continue;
        }
        c += m;
        /*s: [[span()]] pool handling for optab o */
        switch(o->flag & (LFROM|LTO|LPOOL)) {
        case LFROM:
            addpool(p, &p->from);
            break;
        case LTO:
            addpool(p, &p->to);
            break;

        case LPOOL:
            if ((p->scond&C_SCOND) == COND_ALWAYS)
                flushpool(p, 0);
            break;
        }

        if(p->as==AMOVW && p->to.type==D_REG && p->to.reg==REGPC && 
           (p->scond&C_SCOND) == COND_ALWAYS)
            flushpool(p, 0);

        if(blitrl)
            checkpool(p);
        /*e: [[span()]] pool handling for optab o */
    }
    /*s: [[span()]] if large procedure */
    /* need passes to resolve branches */
    if(c - otxt >= 1L<<17)
        bflag = true;
    /*x: [[span()]] if large procedure */
    /*
     * if any procedure is large enough to
     * generate a large SBRA branch, then
     * generate extra passes putting branches
     * around jmps to fix. this is rare.
     */
    while(bflag) {
        DBG("%5.2f span1\n", cputime());
        bflag = false;
        c = INITTEXT;
        for(p = firstp; p != P; p = p->link) {
            /*s: adjust curtext when iterate over instructions p */
            if(p->as == ATEXT)
                curtext = p;
            /*e: adjust curtext when iterate over instructions p */
            p->pc = c;
            o = oplook(p);
            m = o->size;
            if(m == 0) {
                if(p->as == ATEXT) {
                    autosize = p->to.offset + 4;
                    if(p->from.sym != S)
                        p->from.sym->value = c;
                } else {
                    diag("zero-width instruction\n%P", p);
                }
                continue;
            }
            c += m;
        }
    }
    /*e: [[span()]] if large procedure */

    /*s: [[span()]] if string in text segment */
    if(debug['t']) {
        /* 
         * add strings to text segment
         */
        c = rnd(c, 8);
        for(i=0; i<NHASH; i++)
         for(s = hash[i]; s != S; s = s->link) {
            if(s->type != SSTRING)
                continue;
            v = s->value;
            while(v & 3)
                v++;
            s->value = c;
            c += v;
        }
    }
    /*e: [[span()]] if string in text segment */

    c = rnd(c, 8);

    setext = lookup("etext", 0);
    if(setext != S) {
        setext->value = c;
        textsize = c - INITTEXT;
    }
    if(INITRND)
        INITDAT = rnd(c, INITRND);
    DBG("tsize = %lux\n", textsize);
}
/*e: function span(arm) */

/*s: function checkpool(arm) */
/*
 * when the first reference to the literal pool threatens
 * to go out of range of a 12-bit PC-relative offset,
 * drop the pool now, and branch round it.
 * this happens only in extended basic blocks that exceed 4k.
 */
void
checkpool(Prog *p)
{
    if(pool.size >= 0xffc || immaddr((p->pc+4)+4+pool.size - pool.start+8) == 0)
        flushpool(p, 1);
    else if(p->link == P)
        flushpool(p, 2);
}
/*e: function checkpool(arm) */

/*s: function flushpool(arm) */
void
flushpool(Prog *p, int skip)
{
    Prog *q;

    if(blitrl) {
        if(skip){
            if(debug['v'] && skip == 1)
                print("note: flush literal pool at %lux: len=%lud ref=%lux\n", p->pc+4, pool.size, pool.start);
            q = prg();
            q->as = AB;
            q->to.type = D_BRANCH;
            q->cond = p->link;
            q->link = blitrl;
            blitrl = q;
        }
        else if(p->pc+pool.size-pool.start < 2048)
            return;
        elitrl->link = p->link;
        p->link = blitrl;
        blitrl = nil;	/* BUG: should refer back to values until out-of-range */
        elitrl = nil;
        pool.size = 0;
        pool.start = 0;
    }
}
/*e: function flushpool(arm) */

/*s: function addpool(arm) */
void
addpool(Prog *p, Adr *a)
{
    Prog *q, t;
    int c;

    c = aclass(a);

    t = zprg;
    t.as = AWORD;

    switch(c) {
    case C_SROREG:
    case C_LOREG:
    case C_ROREG:
    case C_FOREG:
    case C_SOREG:
    case C_FAUTO:
    case C_SAUTO:
    case C_LAUTO:
    case C_LACON:
        t.to.type = D_CONST;
        t.to.offset = instoffset;
        break;
    default:
        t.to = *a;
        break;
    }

    for(q = blitrl; q != P; q = q->link)	/* could hash on t.t0.offset */
        if(memcmp(&q->to, &t.to, sizeof(t.to)) == 0) {
            p->cond = q;
            return;
        }

    q = prg();
    *q = t;
    q->pc = pool.size;

    if(blitrl == P) {
        blitrl = q;
        pool.start = p->pc;
    } else
        elitrl->link = q;
    elitrl = q;
    pool.size += 4;

    p->cond = q;
}
/*e: function addpool(arm) */

/*s: function xdefine(arm) */
void
xdefine(char *p, int t, long v)
{
    Sym *s;

    s = lookup(p, 0);
    if(s->type == SNONE || s->type == SXREF) {
        s->type = t;
        s->value = v;
    }
}
/*e: function xdefine(arm) */

/*s: function regoff(arm) */
long
regoff(Adr *a)
{

    instoffset = 0;
    aclass(a);
    return instoffset;
}
/*e: function regoff(arm) */

/*s: function immrot(arm) */
long
immrot(ulong v)
{
    int i;

    for(i=0; i<16; i++) {
        if((v & ~0xff) == 0)
            return (i<<8) | v | (1<<25);
        v = (v<<2) | (v>>30);
    }
    return 0;
}
/*e: function immrot(arm) */

/*s: function immaddr(arm) */
long
immaddr(long v)
{
    if(v >= 0 && v <= 0xfff)
        return (v & 0xfff) |
            (1<<24) |	/* pre indexing */
            (1<<23);	/* pre indexing, up */
    if(v >= -0xfff && v < 0)
        return (-v & 0xfff) |
            (1<<24);	/* pre indexing */
    return 0;
}
/*e: function immaddr(arm) */

/*s: function immfloat(arm) */
int
immfloat(long v)
{
    return (v & 0xC03) == 0;	/* offset will fit in floating-point load/store */
}
/*e: function immfloat(arm) */

/*s: function immhalf(arm) */
int
immhalf(long v)
{
    if(v >= 0 && v <= 0xff)
        return v|
            (1<<24)|	/* pre indexing */
            (1<<23);	/* pre indexing, up */
    if(v >= -0xff && v < 0)
        return (-v & 0xff)|
            (1<<24);	/* pre indexing */
    return 0;
}
/*e: function immhalf(arm) */

/*s: function aclass(arm) */
int
aclass(Adr *a)
{
    Sym *s;
    int t;

    switch(a->type) {
    case D_NONE:
        return C_NONE;

    case D_REG:
        return C_REG;
    case D_REGREG:
        return C_REGREG;
    case D_SHIFT:
        return C_SHIFT;
    case D_PSR:
        return C_PSR;

    case D_BRANCH:
        return C_BRANCH;

    /*s: [[aclass()]] switch type cases */
    case D_OREG:
        switch(a->symkind) {
        /*s: [[aclass()]] D_OREG case, switch symkind cases */
        case D_EXTERN:
        case D_STATIC:
            if(a->sym == nil || a->sym->name == nil) {
                print("null sym external\n");
                print("%D\n", a);
                return C_GOK;
            }
            s = a->sym;
            t = s->type;
            if(t == SNONE || t == SXREF) {
                diag("undefined external: %s in %s",
                    s->name, TNAME);
                s->type = SDATA;
            }
            /*s: [[aclass()]] when D_OREG and D_STATIC and dlm */
            if(dlm) {
                switch(t) {
                case SUNDEF:
                case STEXT:
                case SLEAF:
                case SCONST:
                case SSTRING:
                    instoffset = s->value + a->offset;
                    break;
                default:
                    instoffset = s->value + a->offset + INITDAT;
                    break;
                }
                return C_ADDR;
            }
            /*e: [[aclass()]] when D_OREG and D_STATIC and dlm */
            instoffset = s->value + a->offset - BIG;
            t = immaddr(instoffset);
            if(t) {
                if(immhalf(instoffset))
                    return immfloat(t) ? C_HFEXT : C_HEXT;
                if(immfloat(t))
                    return C_FEXT;
                return C_SEXT;
            }
            return C_LEXT;
        /*x: [[aclass()]] D_OREG case, switch symkind cases */
        case D_AUTO:
            instoffset = autosize + a->offset;
            t = immaddr(instoffset);
            if(t){
                if(immhalf(instoffset))
                    return immfloat(t) ? C_HFAUTO : C_HAUTO;
                if(immfloat(t))
                    return C_FAUTO;
                return C_SAUTO;
            }
            return C_LAUTO;
        /*x: [[aclass()]] D_OREG case, switch symkind cases */
        case D_PARAM:
            instoffset = autosize + a->offset + 4L;
            t = immaddr(instoffset);
            if(t){
                if(immhalf(instoffset))
                    return immfloat(t) ? C_HFAUTO : C_HAUTO;
                if(immfloat(t))
                    return C_FAUTO;
                return C_SAUTO;
            }
            return C_LAUTO;
        /*x: [[aclass()]] D_OREG case, switch symkind cases */
        case D_NONE:
            instoffset = a->offset;
            t = immaddr(instoffset);
            if(t) {
                if(immhalf(instoffset))		 /* n.b. that it will also satisfy immrot */
                    return immfloat(t) ? C_HFOREG : C_HOREG;
                if(immfloat(t))
                    return C_FOREG; /* n.b. that it will also satisfy immrot */

                t = immrot(instoffset);
                if(t)
                    return C_SROREG;
                if(immhalf(instoffset))
                    return C_HOREG;
                return C_SOREG;
            }
            t = immrot(instoffset);
            if(t)
                return C_ROREG;
            return C_LOREG;
        /*e: [[aclass()]] D_OREG case, switch symkind cases */
        }
        return C_GOK;
    /*x: [[aclass()]] switch type cases */
    case D_CONST:
        switch(a->symkind) {
        /*s: [[aclass()]] D_CONST case, switch symkind cases */
        case D_NONE:
            instoffset = a->offset;
            if(a->reg != R_NONE)
                goto aconsize;

            t = immrot(instoffset);
            if(t)
                return C_RCON;

            t = immrot(~instoffset);
            if(t)
                return C_NCON;
            return C_LCON;
        /*x: [[aclass()]] D_CONST case, switch symkind cases */
        case D_EXTERN:
        case D_STATIC:
            s = a->sym;
            if(s == S)
                break;
            t = s->type;
            switch(t) {
            case STEXT: case SLEAF:
            case SSTRING:
            case SCONST:
            case SUNDEF:
                instoffset = s->value + a->offset;
                return C_LCON;
            case SNONE: case SXREF:
                diag("undefined external: %s in %s", s->name, TNAME);
                s->type = SDATA;
                break;
            }
            if(!dlm) {
                instoffset = s->value + a->offset - BIG;
                t = immrot(instoffset);
                if(t && instoffset != 0)
                    return C_RECON;
            }
            instoffset = s->value + a->offset + INITDAT;
            return C_LCON;

        /*x: [[aclass()]] D_CONST case, switch symkind cases */
        case D_AUTO:
            instoffset = autosize + a->offset;
            goto aconsize;

        /*x: [[aclass()]] D_CONST case, switch symkind cases */
        case D_PARAM:
            instoffset = autosize + a->offset + 4L;
            goto aconsize;
        /*x: [[aclass()]] D_CONST case, switch symkind cases */
        aconsize:
            t = immrot(instoffset);
            if(t)
                return C_RACON;
            return C_LACON;
        /*e: [[aclass()]] D_CONST case, switch symkind cases */
        }
        return C_GOK;
    /*x: [[aclass()]] switch type cases */
    case D_FREG:
        return C_FREG;
    case D_FCONST:
        return C_FCON;
    case D_FPCR:
        return C_FCR;
    /*e: [[aclass()]] switch type cases */
    }
    return C_GOK;
}
/*e: function aclass(arm) */

/*s: function oplook(arm) */
Optab*
oplook(Prog *p)
{
    int a1, a2, a3, r;
    char *c1, *c3;
    Optab *o, *e;

    a1 = p->optab;
    if(a1)
        return optab+(a1-1);

    a1 = p->from.class;
    if(a1 == C_NONE) {
        a1 = aclass(&p->from) + 1;
        p->from.class = a1;
    }
    a1--;

    a3 = p->to.class;
    if(a3 == C_NONE) {
        a3 = aclass(&p->to) + 1;
        p->to.class = a3;
    }
    a3--;

    a2 = C_NONE;
    if(p->reg != R_NONE)
        a2 = C_REG;

    r = p->as;
    o = oprange[r].start;
    if(o == nil) {
        o = oprange[r].stop; /* just generate an error */
    }
    e = oprange[r].stop;
    c1 = xcmp[a1];
    c3 = xcmp[a3];
    for(; o<e; o++)
        if(o->a2 == a2)
         if(c1[o->a1])
          if(c3[o->a3]) {
            p->optab = (o-optab)+1;
            return o;
        }

    diag("illegal combination %A %d %d %d",
        p->as, a1, a2, a3);
    prasm(p);
    if(o == nil)
        o = optab;
    return o;
}
/*e: function oplook(arm) */

/*s: function cmp(arm) */
int
cmp(int a, int b)
{

    if(a == b)
        return 1;

    switch(a) {
    case C_LCON:
        if(b == C_RCON || b == C_NCON)
            return 1;
        break;
    case C_LACON:
        if(b == C_RACON)
            return 1;
        break;

    case C_HFEXT:
        return b == C_HEXT || b == C_FEXT;
    case C_FEXT:
    case C_HEXT:
        return b == C_HFEXT;
    case C_SEXT:
        return cmp(C_HFEXT, b);
    case C_LEXT:
        return cmp(C_SEXT, b);

    case C_HFAUTO:
        return b == C_HAUTO || b == C_FAUTO;
    case C_FAUTO:
    case C_HAUTO:
        return b == C_HFAUTO;
    case C_SAUTO:
        return cmp(C_HFAUTO, b);
    case C_LAUTO:
        return cmp(C_SAUTO, b);

    case C_HFOREG:
        return b == C_HOREG || b == C_FOREG;
    case C_FOREG:
    case C_HOREG:
        return b == C_HFOREG;
    case C_SROREG:
        return cmp(C_SOREG, b) || cmp(C_ROREG, b);
    case C_SOREG:
    case C_ROREG:
        return b == C_SROREG || cmp(C_HFOREG, b);
    case C_LOREG:
        return cmp(C_SROREG, b);

    }
    return 0;
}
/*e: function cmp(arm) */

/*s: function ocmp(arm) */
int
ocmp(const void *a1, const void *a2)
{
    Optab *p1, *p2;
    int n;

    p1 = (Optab*)a1;
    p2 = (Optab*)a2;
    n = p1->as - p2->as;
    if(n)
        return n;

    n = (p2->flag&V4) - (p1->flag&V4);	/* architecture version */
    if(n)
        return n;
    n = (p2->flag&VFP) - (p1->flag&VFP);	/* floating point arch */
    if(n)
        return n;

    n = p1->a1 - p2->a1;
    if(n)
        return n;
    n = p1->a2 - p2->a2;
    if(n)
        return n;
    n = p1->a3 - p2->a3;
    if(n)
        return n;
    return 0;
}
/*e: function ocmp(arm) */

/*s: function buildop(arm) */
void
buildop(void)
{
    int i, n, r;

    armv4 = !debug['h'];
    vfp = debug['f'];

    for(i=0; i<C_GOK; i++)
        for(n=0; n<C_GOK; n++)
            xcmp[i][n] = cmp(n, i);

    for(n=0; optab[n].as != AXXX; n++) {
        if((optab[n].flag & VFP) && !vfp)
            optab[n].as = AXXX;
        if((optab[n].flag & V4) && !armv4) {
            optab[n].as = AXXX;
            break;
        }
    }

    qsort(optab, n, sizeof(optab[0]), ocmp);
    for(i=0; i<n; i++) {
        r = optab[i].as;

        oprange[r].start = optab+i;
        while(optab[i].as == r)
            i++;
        oprange[r].stop = optab+i;
        i--;

        switch(r)
        {
        /*s: [[buildop()]] switch opcode r cases */
        case AXXX:
            break;
        case AADD:
            oprange[AAND] = oprange[r];
            oprange[AEOR] = oprange[r];
            oprange[ASUB] = oprange[r];
            oprange[ARSB] = oprange[r];
            oprange[AADC] = oprange[r];
            oprange[ASBC] = oprange[r];
            oprange[ARSC] = oprange[r];
            oprange[AORR] = oprange[r];
            oprange[ABIC] = oprange[r];
            break;
        case ACMP:
            oprange[ATST] = oprange[r];
            oprange[ATEQ] = oprange[r];
            oprange[ACMN] = oprange[r];
            break;
        case AMVN:
            break;
        case AB:
        case ABL:
            break;
        case ABEQ:
            oprange[ABNE] = oprange[r];
            oprange[ABCS] = oprange[r];
            oprange[ABHS] = oprange[r];
            oprange[ABCC] = oprange[r];
            oprange[ABLO] = oprange[r];
            oprange[ABMI] = oprange[r];
            oprange[ABPL] = oprange[r];
            oprange[ABVS] = oprange[r];
            oprange[ABVC] = oprange[r];
            oprange[ABHI] = oprange[r];
            oprange[ABLS] = oprange[r];
            oprange[ABGE] = oprange[r];
            oprange[ABLT] = oprange[r];
            oprange[ABGT] = oprange[r];
            oprange[ABLE] = oprange[r];
            break;
        case ASLL:
            oprange[ASRL] = oprange[r];
            oprange[ASRA] = oprange[r];
            break;
        case AMUL:
            oprange[AMULU] = oprange[r];
            break;
        case ADIV:
            oprange[AMOD] = oprange[r];
            oprange[AMODU] = oprange[r];
            oprange[ADIVU] = oprange[r];
            break;
        case AMOVW:
        case AMOVB:
        case AMOVBU:
        case AMOVH:
        case AMOVHU:
            break;
        case ASWPW:
            oprange[ASWPBU] = oprange[r];
            break;
        case ASWI:
        case ARFE:
        case ATEXT:
        case AWORD:
        case ACASE:
        case ABCASE:
        case AMOVM:
            break;
        case AMULL:
            oprange[AMULA] = oprange[r];
            oprange[AMULAL] = oprange[r];
            oprange[AMULLU] = oprange[r];
            oprange[AMULALU] = oprange[r];
            break;
        /*x: [[buildop()]] switch opcode r cases */
        case AADDF:
            oprange[AADDD] = oprange[r];
            oprange[ASUBF] = oprange[r];
            oprange[ASUBD] = oprange[r];
            oprange[AMULF] = oprange[r];
            oprange[AMULD] = oprange[r];
            oprange[ADIVF] = oprange[r];
            oprange[ADIVD] = oprange[r];
            oprange[AMOVFD] = oprange[r];
            oprange[AMOVDF] = oprange[r];
            break;
    
        case ACMPF:
            oprange[ACMPD] = oprange[r];
            break;

        case AMOVF:
            oprange[AMOVD] = oprange[r];
            break;

        case AMOVFW:
            oprange[AMOVWF] = oprange[r];
            oprange[AMOVWD] = oprange[r];
            oprange[AMOVDW] = oprange[r];
            break;
        /*e: [[buildop()]] switch opcode r cases */
        default:
            diag("unknown op in build: %A", r);
            errorexit();
        }
    }
}
/*e: function buildop(arm) */

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
/*e: linkers/5l/span.c */
