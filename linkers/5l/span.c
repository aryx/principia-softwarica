/*s: linkers/5l/span.c */
#include	"l.h"
#include	"m.h"

// forward decls
long    immaddr(long);

/*s: global pool(arm) */
static struct {
    // PC of first instruction referencing the pool
    ulong	start;
    // a multiple of 4
    ulong	size;
} pool;
/*e: global pool(arm) */

// forward decls
void	checkpool(Prog*);
void 	flushpool(Prog*, int);
void    addpool(Prog*, Adr*);
typedef struct Reloc Reloc;
int cmp(int, int);
int     ocmp(const void*, const void*);

/*s: function span(arm) */
/// main -> <>
void
dotext(void)
{
    Prog *p;
    Optab *o;
    // real code address
    long c; // ulong?
    // size of instruction
    int m;
    /*s: [[dotext()]] other locals */
    long    otxt;
    /*x: [[dotext()]] other locals */
    Sym *s;
    long v;
    int i;
    /*e: [[dotext()]] other locals */

    DBG("%5.2f span\n", cputime());

    c = INITTEXT;
    /*s: [[dotext()]] initialisation */
    otxt = c;
    /*e: [[dotext()]] initialisation */

    for(p = firstp; p != P; p = p->link) {
       /*s: adjust curtext when iterate over instructions p */
       if(p->as == ATEXT)
           curtext = p;
       /*e: adjust curtext when iterate over instructions p */
       /*s: adjust autosize when iterate over instructions p */
       if(p->as == ATEXT) {
           autosize = p->to.offset + 4;
       }
       /*e: adjust autosize when iterate over instructions p */

        // real program counter transition
        p->pc = c;

        o = oplook(p);
        m = o->size;
        c += m;

        if(m == 0) {
            if(p->as == ATEXT) {
                if(p->from.sym != S)
                    p->from.sym->value = c;
                /*s: [[dotext()]] detect if large procedure */
                if(c - otxt >= 1L<<17) {
                    diag("Procedure %s too large\n", TNAME);
                    errorexit();
                }
                otxt = c;
                /*e: [[dotext()]] detect if large procedure */
            } else {
                diag("zero-width instruction\n%P", p);
            }
        } else {
            /*s: [[dotext()]] pool handling for optab o */
            switch(o->flag & (LFROM|LTO|LPOOL)) {
            /*s: [[dotext()]] pool handling, switch flag cases */
            case LFROM:
                addpool(p, &p->from);
                break;
            case LTO:
                addpool(p, &p->to);
                break;
            /*x: [[dotext()]] pool handling, switch flag cases */
            case LPOOL:
                if ((p->scond&C_SCOND) == COND_ALWAYS)
                    flushpool(p, false);
                break;
            /*e: [[dotext()]] pool handling, switch flag cases */
            }
            /*s: [[dotext()]] pool handling, flush if MOVW REGPC */
            // MOVW ..., R15  => flush
            if(p->as==AMOVW && p->to.type==D_REG && p->to.reg==REGPC && 
               (p->scond&C_SCOND) == COND_ALWAYS)
                flushpool(p, false);
            /*e: [[dotext()]] pool handling, flush if MOVW REGPC */
            /*s: [[dotext()]] pool handling, checkpool */
            if(blitrl)
                checkpool(p);
            /*e: [[dotext()]] pool handling, checkpool */
            /*e: [[dotext()]] pool handling for optab o */
        }
    }

    /*s: [[dotext()]] if string in text segment */
    if(debug['t']) {
        /* 
         * add strings to text segment
         */
        c = rnd(c, 8);
        for(i=0; i<NHASH; i++)
         for(s = hash[i]; s != S; s = s->link)
          if(s->type == SSTRING) {
              v = s->value;
              v = rnd(v, 4);
              s->value = c;
              c += v;
          }
    }
    /*e: [[dotext()]] if string in text segment */

    c = rnd(c, 8);

    textsize = c - INITTEXT;
    /*s: [[dotext()]] refine special symbols */
    lookup("etext", 0)->value = INITTEXT+textsize;
    /*e: [[dotext()]] refine special symbols */
    if(INITRND)
        INITDAT = rnd(c, INITRND);
    DBG("tsize = %lux\n", textsize);
}
/*e: function span(arm) */

/*s: function checkpool(arm) */
void
checkpool(Prog *p)
{
    if(p->link == P)
        flushpool(p, true);
    else
    /*s: [[checkpool()]] if special condition */
    /*
     * When the first reference to the literal pool threatens
     * to go out of range of a 12-bit PC-relative offset,
     * drop the pool now, and branch round it.
     * This happens only in extended basic blocks that exceed 4k.
     */
    if(pool.size >= 0xffc || 
       immaddr((p->pc+4) + 4 + pool.size - pool.start + 8) == 0)
        flushpool(p, true);
    /*e: [[checkpool()]] if special condition */
}
/*e: function checkpool(arm) */

/*s: function flushpool(arm) */
/// (dotext -> checkpool) | dotext -> <>
void
flushpool(Prog *p, bool skip)
{
    Prog *q;

    if(blitrl) {
        /*s: [[flushpool()]] if skip or corner case */
        if(skip){
            DBG("note: flush literal pool at %lux: len=%lud ref=%lux\n", 
                p->pc+4, pool.size, pool.start);
            q = prg();
            q->as = AB;
            q->to.type = D_BRANCH;
            q->cond = p->link;
    
            //insert_list(q, blitrl)
            q->link = blitrl;
            blitrl = q;
        }
        /*s: [[flushpool()]] else if not skip and corner case */
        else if((p->pc + pool.size - pool.start) < 2048)
            return;
        /*e: [[flushpool()]] else if not skip and corner case */
        /*e: [[flushpool()]] if skip or corner case */

        //insert_list_after_elt(blitlr, elitrl, p)
        elitrl->link = p->link;
        p->link = blitrl;

        blitrl = nil;/* BUG: should refer back to values until out-of-range */
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
    Prog *q;
    Prog t;
    //enum<Operand_class>
    int c;

    c = aclass(a);

    t = zprg;
    t.as = AWORD;
    /*s: [[addpool()]] set t.to using a */
    switch(c) {
    /*s: [[addpool()]] switch operand class [[c]] cases */
    // C_LCON|C_xCON, C_LEXT|C_xEXT? TODO warning if other case?
    default:
        t.to = *a;
        break;
    /*x: [[addpool()]] switch operand class [[c]] cases */
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
    /*e: [[addpool()]] switch operand class [[c]] cases */
    }
    /*e: [[addpool()]] set t.to using a */

    /*s: [[addpool()]] if literal already present in pool */
    // find_list(t.to, blitrl)
    for(q = blitrl; q != P; q = q->link)
        if(memcmp(&q->to, &t.to, sizeof(Adr)) == 0) {
            // for omvl()
            p->cond = q;
            return;
        }
    /*e: [[addpool()]] if literal already present in pool */
    // else

    q = prg();
    *q = t;

    /*s: [[addpool()]] set pool.start and pool.size */
    // will be overwritten when dotext() layout the pool later
    q->pc = pool.size;

    if(blitrl == P) {
        pool.start = p->pc;
    }
    pool.size += 4;
    /*e: [[addpool()]] set pool.start and pool.size */

    // add_queue(q, blitrl, elitrl)
    if(blitrl == P) {
        blitrl = q;
    } else
        elitrl->link = q;
    elitrl = q;

    // for omvl()!
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
// ulong -> option<long> (None = 0)
long
immrot(ulong v)
{
    // the rotation
    int i;

    for(i=0; i<16; i++) {
        if((v & ~0xff) == 0)
            //      i,r,r opcodes       rotation     number        
            return  (1<<25)         |    (i<<8)   |   v     ;
        // inverse of 2 bits rotation to the right
        v = (v<<2) | 
            (v>>30);
    }
    return 0;
}
/*e: function immrot(arm) */

/*s: function immaddr(arm) */
// ulong -> option<long> (None = 0)
long
immaddr(long v)
{
    if(v >= 0 && v <= 0xfff)
        return (v & 0xfff) |
            (1<<24) |	/* pre indexing */
            (1<<23);	/* up */

    if(v < 0 && v >= -0xfff)
        return (-v & 0xfff) |
            (1<<24);	/* pre indexing */
    return 0;
}
/*e: function immaddr(arm) */

/*s: function immfloat(arm) */
static int
immfloat(long v)
{
    return (v & 0xC03) == 0;/* offset will fit in floating-point load/store */
}
/*e: function immfloat(arm) */

/*s: function immhalf(arm) */
static int
immhalf(long v)
{
    if(v >= 0 && v <= 0xff)
        return v |
            (1<<24)  |	/* pre indexing */
            (1<<23);	/* pre indexing, up */
    if(v >= -0xff && v < 0)
        return (-v & 0xff)|
            (1<<24);	/* pre indexing */
    return 0;
}
/*e: function immhalf(arm) */

/*s: function aclass(arm) */
// oplook | ... -> <>
int
aclass(Adr *a)
{
    /*s: [[aclass()]] locals */
    Sym *s;
    // enum<Section>
    int t;
    /*e: [[aclass()]] locals */

    switch(a->type) {
    /*s: [[aclass()]] switch type cases */
    case D_NONE:
        return C_NONE;
    case D_REG:
        return C_REG;
    case D_BRANCH:
        return C_BRANCH;
    /*x: [[aclass()]] switch type cases */
    case D_CONST:
        instoffset = a->offset;
        if(a->reg != R_NONE) // when??????
            goto aconsize;

        if(immrot(instoffset))
            return C_RCON;
        if(immrot(~instoffset))
            return C_NCON;
        return C_LCON;
    /*x: [[aclass()]] switch type cases */
    case D_OREG:
        switch(a->symkind) {
        /*s: [[aclass()]] D_OREG case, switch symkind cases */
        case N_NONE:
            instoffset = a->offset;
            t = immaddr(instoffset);
            if(t) {
                /*s: [[aclass()]] if immfloat for N_NONE symbol */
                if(immfloat(t))
                    return immhalf(instoffset)? C_HFOREG : C_FOREG;
                    /* n.b. that [C_FOREG] will also satisfy immrot */
                /*e: [[aclass()]] if immfloat for N_NONE symbol */
                /*s: [[aclass()]] if immhalf for N_NONE symbol */
                 /* n.b. that immhalf() will also satisfy immrot */
                if(immhalf(instoffset))	
                    return C_HOREG;
                /*e: [[aclass()]] if immhalf for N_NONE symbol */
                if(immrot(instoffset))
                    return C_SROREG;
                return C_SOREG;
            }
            if(immrot(instoffset))
                return C_ROREG;
            return C_LOREG;
        /*x: [[aclass()]] D_OREG case, switch symkind cases */
        case N_EXTERN:
        case N_INTERN:
            /*s: [[aclass()]] D_OREG case, N_EXTERN case, sanity check a */
            if(a->sym == nil || a->sym->name == nil) {
                print("null sym external\n");
                print("%D\n", a);
                return C_GOK;
            }
            /*e: [[aclass()]] D_OREG case, N_EXTERN case, sanity check a */
            s = a->sym;
            t = s->type;
            /*s: [[aclass()]] D_OREG case, N_EXTERN case, sanity check t */
            if(t == SNONE || t == SXREF) {
                diag("undefined external: %s in %s", s->name, TNAME);
                s->type = SDATA;
            }
            /*e: [[aclass()]] D_OREG case, N_EXTERN case, sanity check t */
            /*s: [[aclass()]] when D_OREG and external symbol and dlm */
            if(dlm) {
                switch(t) {
                case STEXT: case SSTRING:
                case SUNDEF:
                    instoffset = s->value + a->offset;
                    break;
                case SDATA: case SBSS:
                default:
                    instoffset = s->value + a->offset + INITDAT;
                    break;
                }
                return C_ADDR;
            }
            /*e: [[aclass()]] when D_OREG and external symbol and dlm */
            instoffset = s->value + a->offset - BIG;
            t = immaddr(instoffset);
            if(t) {
                /*s: [[aclass()]] if immfloat for N_EXTERN symbol */
                if(immfloat(t))
                    return immhalf(instoffset)? C_HFEXT : C_FEXT;
                /*e: [[aclass()]] if immfloat for N_EXTERN symbol */
                /*s: [[aclass()]] if immhalf for N_EXTERN symbol */
                if(immhalf(instoffset))
                    return C_HEXT;
                /*e: [[aclass()]] if immhalf for N_EXTERN symbol */
                return C_SEXT;
            }
            return C_LEXT;
        /*x: [[aclass()]] D_OREG case, switch symkind cases */
        case N_LOCAL:
            instoffset = autosize + a->offset;
            t = immaddr(instoffset);
            if(t){
                /*s: [[aclass()]] if immfloat for N_LOCAL or N_PARAM symbol */
                if(immfloat(t))
                    return immhalf(instoffset)? C_HFAUTO : C_FAUTO;
                /*e: [[aclass()]] if immfloat for N_LOCAL or N_PARAM symbol */
                /*s: [[aclass()]] if immhalf for N_LOCAL or N_PARAM symbol */
                if(immhalf(instoffset))
                    return C_HAUTO;
                /*e: [[aclass()]] if immhalf for N_LOCAL or N_PARAM symbol */
                return C_SAUTO;
            }
            return C_LAUTO;
        /*x: [[aclass()]] D_OREG case, switch symkind cases */
        case N_PARAM:
            instoffset = autosize + 4L + a->offset;
            t = immaddr(instoffset);
            if(t){
                /*s: [[aclass()]] if immfloat for N_LOCAL or N_PARAM symbol */
                if(immfloat(t))
                    return immhalf(instoffset)? C_HFAUTO : C_FAUTO;
                /*e: [[aclass()]] if immfloat for N_LOCAL or N_PARAM symbol */
                /*s: [[aclass()]] if immhalf for N_LOCAL or N_PARAM symbol */
                if(immhalf(instoffset))
                    return C_HAUTO;
                /*e: [[aclass()]] if immhalf for N_LOCAL or N_PARAM symbol */
                return C_SAUTO;
            }
            return C_LAUTO;
        /*e: [[aclass()]] D_OREG case, switch symkind cases */
        }
        return C_GOK;
    /*x: [[aclass()]] switch type cases */
    case D_ADDR:
        switch(a->symkind) {
        /*s: [[aclass()]] D_ADDR case, switch symkind cases */
        case N_EXTERN:
        case N_INTERN:
            s = a->sym;
            /*s: [[aclass()]] D_ADDR case, N_EXTERN case, sanity check s */
            if(s == S) // no warning?
                break;
            /*e: [[aclass()]] D_ADDR case, N_EXTERN case, sanity check s */
            switch(s->type) {
            case STEXT: case SSTRING: case SUNDEF:
                instoffset = s->value + a->offset;
                return C_LCON; // etext is stable
            case SNONE: case SXREF:
                diag("undefined external: %s in %s", s->name, TNAME);
                s->type = SDATA;
                // Fall through
            case SDATA: case SBSS: case SDATA1:
                /*s: [[aclass()]] in D_ADDR case, SDATA case, if dlm */
                if(dlm) {
                    instoffset = s->value + a->offset + INITDAT;
                    return C_LCON;
                }
                /*e: [[aclass()]] in D_ADDR case, SDATA case, if dlm */
                instoffset = s->value + a->offset - BIG;
                if(immrot(instoffset) && instoffset != 0) {// VERY IMPORTANT != 0
                     return C_RECON;
                } else {
                    instoffset = s->value + a->offset + INITDAT;
                    return C_LCON;
                }
            }
            diag("unknown section for %s", s->name);
            break;
        /*x: [[aclass()]] D_ADDR case, switch symkind cases */
        case N_LOCAL:
            instoffset = autosize + a->offset;
            goto aconsize;
        /*x: [[aclass()]] D_ADDR case, switch symkind cases */
        case N_PARAM:
            instoffset = autosize + a->offset + 4L;
            goto aconsize;
        /*x: [[aclass()]] D_ADDR case, switch symkind cases */
        aconsize:
            return immrot(instoffset)? C_RACON : C_LACON;
        /*e: [[aclass()]] D_ADDR case, switch symkind cases */
        }
        return C_GOK;
    /*x: [[aclass()]] switch type cases */
    case D_SHIFT:
        return C_SHIFT;
    /*x: [[aclass()]] switch type cases */
    case D_FREG:
        return C_FREG;
    case D_FCONST:
        return C_FCON;
    case D_FPCR:
        return C_FCR;
    /*x: [[aclass()]] switch type cases */
    case D_REGREG:
        return C_REGREG;
    /*x: [[aclass()]] switch type cases */
    case D_PSR:
        return C_PSR;
    /*e: [[aclass()]] switch type cases */
    }
    return C_GOK;
}
/*e: function aclass(arm) */

/*s: function oplook(arm) */
/// main -> (dotext | asmb) -> <>
Optab*
oplook(Prog *p)
{
    // enum<Opcode> (to index oprange[])
    int r;
    // enum<Operand_class>
    int a1, a2, a3;
    // ref<Optab> (from = optab)
    Optab *o, *e;
    /*s: [[oplook()]] other locals */
    bool useopti = true;
    int i;
    /*x: [[oplook()]] other locals */
    bool *c1, *c3;
    /*e: [[oplook()]] other locals */
 
    /*s: [[oplook()]] if use opti, part1 */
    if(useopti) {
        i = p->optab;
        if(i)
            return optab+(i-1);
    
        a1 = p->from.class;
        if(a1 == 0) {
            a1 = aclass(&p->from) + 1;
            p->from.class = a1;
        }
        a1--;
    
        a3 = p->to.class;
        if(a3 == 0) {
            a3 = aclass(&p->to) + 1;
            p->to.class = a3;
        }
        a3--;
    }
    /*e: [[oplook()]] if use opti, part1 */
    else {
        a1 = aclass(&p->from);
        a3 = aclass(&p->to);
    }
    a2 = (p->reg != R_NONE)? C_REG : C_NONE;
    r = p->as;

    // find the range of relevant optab entries
    o = oprange[r].start;
    e = oprange[r].stop;
    /*s: [[oplook()]] sanity check o */
    if(o == nil) {
        o = oprange[r].stop; /* just generate an error */
    }
    /*e: [[oplook()]] sanity check o */

    /*s: [[oplook()]] debug */
    if(debug['t']) {
        print("oplook %P %A %d %d %d\n",
            p, (int)p->as, a1, a2, a3);
    }
    /*e: [[oplook()]] debug */

    /*s: [[oplook()]] if use opti, part2 */
    if(useopti) {
        c1 = xcmp[a1];
        c3 = xcmp[a3];
        for(; o<e; o++)
            if(o->a2 == a2)
             if(c1[o->a1])
              if(c3[o->a3]) {
                /*s: [[oplook()]] if use opti, in part2, memoize matching rule [[o]] */
                p->optab = (o-optab)+1;
                /*e: [[oplook()]] if use opti, in part2, memoize matching rule [[o]] */
                return o;
            }
    }
    /*e: [[oplook()]] if use opti, part2 */
    else {
        // iterate over the range
        for(; o<e; o++)
            // compare the operands
            if(o->a2 == a2)
             if(cmp(o->a1, a1))
              if(cmp(o->a3, a3)) {
                // found a matching rule!
                return o;
            }
    }
    /*s: [[oplook()]] illegal combination error */
    diag("illegal combination %A %d %d %d",  p->as, a1, a2, a3);
    prasm(p);
    if(o == nil)
        o = optab;
    return o;
    /*e: [[oplook()]] illegal combination error */
}
/*e: function oplook(arm) */

/*s: function cmp(arm) */
bool
cmp(int a, int b)
{

    if(a == b)
        return true;

    switch(a) {
    /*s: [[cmp()]] switch on a, the operand class in optab rule, cases */
    case C_LCON:
        if(b == C_RCON || b == C_NCON)
            return true;
        break;
    /*x: [[cmp()]] switch on a, the operand class in optab rule, cases */
    case C_SROREG:
        return cmp(C_SOREG, b) || cmp(C_ROREG, b);
    case C_SOREG:
    case C_ROREG:
        return b == C_SROREG || cmp(C_HFOREG, b);
    case C_LOREG:
        return cmp(C_SROREG, b);
    /*x: [[cmp()]] switch on a, the operand class in optab rule, cases */
    case C_SEXT:
        return cmp(C_HFEXT, b);
    case C_LEXT:
        return cmp(C_SEXT, b);
    /*x: [[cmp()]] switch on a, the operand class in optab rule, cases */
    case C_SAUTO:
        return cmp(C_HFAUTO, b);
    case C_LAUTO:
        return cmp(C_SAUTO, b);
    /*x: [[cmp()]] switch on a, the operand class in optab rule, cases */
    case C_LACON:
        if(b == C_RACON)
            return true;
        break;
    /*x: [[cmp()]] switch on a, the operand class in optab rule, cases */
    case C_HFEXT:
        return b == C_HEXT || b == C_FEXT;
    case C_FEXT:
    case C_HEXT:
        return b == C_HFEXT;
    /*x: [[cmp()]] switch on a, the operand class in optab rule, cases */
    case C_HFAUTO:
        return b == C_HAUTO || b == C_FAUTO;
    case C_FAUTO:
    case C_HAUTO:
        return b == C_HFAUTO;
    /*x: [[cmp()]] switch on a, the operand class in optab rule, cases */
    case C_HFOREG:
        return b == C_HOREG || b == C_FOREG;
    case C_FOREG:
    case C_HOREG:
        return b == C_HFOREG;
    /*e: [[cmp()]] switch on a, the operand class in optab rule, cases */
    }
    return false;
}
/*e: function cmp(arm) */

/*s: function ocmp(arm) */
/// main -> buildop -> qsort(..., <>)
int
ocmp(const void *a, const void *b)
{
    Optab *p1, *p2;
    int n;

    p1 = (Optab*)a;
    p2 = (Optab*)b;

    n = p1->as - p2->as;
    if(n)
        return n;
    /*s: [[ocmp()]] if v4 flag on p1 or p2 */
    n = (p2->flag&V4) - (p1->flag&V4);	/* architecture version */
    if(n)
        return n;
    /*e: [[ocmp()]] if v4 flag on p1 or p2 */
    /*s: [[ocmp()]] if floating point flag on p1 or p2 */
    n = (p2->flag&VFP) - (p1->flag&VFP);	/* floating point arch */
    if(n)
        return n;
    /*e: [[ocmp()]] if floating point flag on p1 or p2 */
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
/// main -> <>
void
buildop(void)
{
    int i, n;
    // enum<Opcode> representing a range
    int r;

    /*s: [[buildop()]] initialize xcmp cache */
    for(i=0; i<C_GOK; i++)
        for(n=0; n<C_GOK; n++)
            xcmp[i][n] = cmp(n, i);
    /*e: [[buildop()]] initialize xcmp cache */
    /*s: [[buildop()]] initialize flags */
    vfp = debug['f'];
    /*x: [[buildop()]] initialize flags */
    armv4 = !debug['h'];
    /*e: [[buildop()]] initialize flags */

    for(n=0; optab[n].as != AXXX; n++) {
        /*s: [[buildop()]] adjust optab if flags, remove certain rules */
        if((optab[n].flag & VFP) && !vfp)
            optab[n].as = AXXX;
        /*x: [[buildop()]] adjust optab if flags, remove certain rules */
        if((optab[n].flag & V4) && !armv4) {
            optab[n].as = AXXX;
            break;
        }
        /*e: [[buildop()]] adjust optab if flags, remove certain rules */
    }
    // n contains now the size of optab

    qsort(optab, n, sizeof(optab[0]), ocmp);

    for(i=0; i<n; i++) {
        r = optab[i].as;

        // initialize oprange for the representants
        oprange[r].start = optab+i;
        while(optab[i].as == r)
            i++;
        oprange[r].stop = optab+i;
        i--; // compensate the i++ of the for

        // setup opcode sets of representants
        switch(r)
        {
        /*s: [[buildop()]] switch opcode r for ranges cases */
        case AXXX:
          break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case ATEXT:
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case AWORD:
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case AADD:
            oprange[ASUB] = oprange[r];

            oprange[AAND] = oprange[r];
            oprange[AEOR] = oprange[r];
            oprange[AORR] = oprange[r];

            oprange[AADC] = oprange[r];
            oprange[ASBC] = oprange[r];
            oprange[ARSC] = oprange[r];
            oprange[ARSB] = oprange[r];
            oprange[ABIC] = oprange[r];
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case ACMP:
            oprange[ATST] = oprange[r];
            oprange[ATEQ] = oprange[r];
            oprange[ACMN] = oprange[r];
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case AMVN:
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case ASLL:
            oprange[ASRL] = oprange[r];
            oprange[ASRA] = oprange[r];
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case AMUL:
            oprange[AMULU] = oprange[r];
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case AB:
        case ABL:
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case ABEQ:
            oprange[ABNE] = oprange[r];
            oprange[ABHS] = oprange[r];
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
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case AMOVW:
        case AMOVB:
        case AMOVBU:
        case AMOVH:
        case AMOVHU:
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case ASWPW:
            oprange[ASWPBU] = oprange[r];
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case ASWI:
        case ARFE:
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
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
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case ADIV:
            oprange[AMOD] = oprange[r];
            oprange[AMODU] = oprange[r];
            oprange[ADIVU] = oprange[r];
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case AMULL:
            oprange[AMULA] = oprange[r];
            oprange[AMULAL] = oprange[r];
            oprange[AMULLU] = oprange[r];
            oprange[AMULALU] = oprange[r];
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case AMOVM:
            break;
        /*x: [[buildop()]] switch opcode r for ranges cases */
        case ACASE:
        case ABCASE:
            break;
        /*e: [[buildop()]] switch opcode r for ranges cases */
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
