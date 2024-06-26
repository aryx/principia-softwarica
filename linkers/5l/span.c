/*s: linkers/5l/span.c */
#include	"l.h"
#include	"m.h"

/*s: global [[oprange]](arm) */
// map<enum<Opcode>, Oprange>
Oprange	oprange[ALAST];
/*e: global [[oprange]](arm) */
/*s: global [[xcmp]](arm) */
bool	xcmp[C_GOK+1][C_GOK+1];
/*e: global [[xcmp]](arm) */

/*s: function [[cmp]](arm) */
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
/*e: function [[cmp]](arm) */

/*s: function [[ocmp]](arm) */
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
/*e: function [[ocmp]](arm) */

/*s: function [[buildop]](arm) */
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
/*e: function [[buildop]](arm) */


/*s: function [[regoff]](arm) */
long
regoff(Adr *a)
{

    instoffset = 0;
    aclass(a);
    return instoffset;
}
/*e: function [[regoff]](arm) */

/*s: function [[immrot]](arm) */
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
/*e: function [[immrot]](arm) */

/*s: function [[immaddr]](arm) */
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
/*e: function [[immaddr]](arm) */

/*s: function [[immfloat]](arm) */
static int
immfloat(long v)
{
    return (v & 0xC03) == 0;/* offset will fit in floating-point load/store */
}
/*e: function [[immfloat]](arm) */

/*s: function [[immhalf]](arm) */
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
/*e: function [[immhalf]](arm) */

/*s: function [[aclass]](arm) */
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
        /*s: [[aclass()]] [[D_OREG]] case, switch symkind cases */
        case N_NONE:
            instoffset = a->offset;
            t = immaddr(instoffset);
            if(t) {
                /*s: [[aclass()]] if immfloat for [[N_NONE]] symbol */
                if(immfloat(t))
                    return immhalf(instoffset)? C_HFOREG : C_FOREG;
                    /* n.b. that [C_FOREG] will also satisfy immrot */
                /*e: [[aclass()]] if immfloat for [[N_NONE]] symbol */
                /*s: [[aclass()]] if immhalf for [[N_NONE]] symbol */
                 /* n.b. that immhalf() will also satisfy immrot */
                if(immhalf(instoffset))	
                    return C_HOREG;
                /*e: [[aclass()]] if immhalf for [[N_NONE]] symbol */
                if(immrot(instoffset))
                    return C_SROREG;
                return C_SOREG;
            }
            if(immrot(instoffset))
                return C_ROREG;
            return C_LOREG;
        /*x: [[aclass()]] [[D_OREG]] case, switch symkind cases */
        case N_EXTERN:
        case N_INTERN:
            /*s: [[aclass()]] [[D_OREG]] case, [[N_EXTERN]] case, sanity check a */
            if(a->sym == nil || a->sym->name == nil) {
                print("null sym external\n");
                print("%D\n", a);
                return C_GOK;
            }
            /*e: [[aclass()]] [[D_OREG]] case, [[N_EXTERN]] case, sanity check a */
            s = a->sym;
            t = s->type;
            /*s: [[aclass()]] [[D_OREG]] case, [[N_EXTERN]] case, sanity check t */
            if(t == SNONE || t == SXREF) {
                diag("undefined external: %s in %s", s->name, TNAME);
                s->type = SDATA;
            }
            /*e: [[aclass()]] [[D_OREG]] case, [[N_EXTERN]] case, sanity check t */
            /*s: [[aclass()]] when [[D_OREG]] and external symbol and dlm */
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
            /*e: [[aclass()]] when [[D_OREG]] and external symbol and dlm */
            instoffset = s->value + a->offset - BIG;
            t = immaddr(instoffset);
            if(t) {
                /*s: [[aclass()]] if immfloat for [[N_EXTERN]] symbol */
                if(immfloat(t))
                    return immhalf(instoffset)? C_HFEXT : C_FEXT;
                /*e: [[aclass()]] if immfloat for [[N_EXTERN]] symbol */
                /*s: [[aclass()]] if immhalf for [[N_EXTERN]] symbol */
                if(immhalf(instoffset))
                    return C_HEXT;
                /*e: [[aclass()]] if immhalf for [[N_EXTERN]] symbol */
                return C_SEXT;
            }
            return C_LEXT;
        /*x: [[aclass()]] [[D_OREG]] case, switch symkind cases */
        case N_LOCAL:
            instoffset = autosize + a->offset;
            t = immaddr(instoffset);
            if(t){
                /*s: [[aclass()]] if immfloat for [[N_LOCAL]] or [[N_PARAM]] symbol */
                if(immfloat(t))
                    return immhalf(instoffset)? C_HFAUTO : C_FAUTO;
                /*e: [[aclass()]] if immfloat for [[N_LOCAL]] or [[N_PARAM]] symbol */
                /*s: [[aclass()]] if immhalf for [[N_LOCAL]] or [[N_PARAM]] symbol */
                if(immhalf(instoffset))
                    return C_HAUTO;
                /*e: [[aclass()]] if immhalf for [[N_LOCAL]] or [[N_PARAM]] symbol */
                return C_SAUTO;
            }
            return C_LAUTO;
        /*x: [[aclass()]] [[D_OREG]] case, switch symkind cases */
        case N_PARAM:
            instoffset = autosize + 4L + a->offset;
            t = immaddr(instoffset);
            if(t){
                /*s: [[aclass()]] if immfloat for [[N_LOCAL]] or [[N_PARAM]] symbol */
                if(immfloat(t))
                    return immhalf(instoffset)? C_HFAUTO : C_FAUTO;
                /*e: [[aclass()]] if immfloat for [[N_LOCAL]] or [[N_PARAM]] symbol */
                /*s: [[aclass()]] if immhalf for [[N_LOCAL]] or [[N_PARAM]] symbol */
                if(immhalf(instoffset))
                    return C_HAUTO;
                /*e: [[aclass()]] if immhalf for [[N_LOCAL]] or [[N_PARAM]] symbol */
                return C_SAUTO;
            }
            return C_LAUTO;
        /*e: [[aclass()]] [[D_OREG]] case, switch symkind cases */
        }
        return C_GOK;
    /*x: [[aclass()]] switch type cases */
    case D_ADDR:
        switch(a->symkind) {
        /*s: [[aclass()]] [[D_ADDR]] case, switch symkind cases */
        case N_EXTERN:
        case N_INTERN:
            s = a->sym;
            /*s: [[aclass()]] [[D_ADDR]] case, [[N_EXTERN]] case, sanity check s */
            if(s == S) // no warning?
                break;
            /*e: [[aclass()]] [[D_ADDR]] case, [[N_EXTERN]] case, sanity check s */
            switch(s->type) {
            case STEXT: case SSTRING: case SUNDEF:
                instoffset = s->value + a->offset;
                return C_LCON; // etext is stable
            case SNONE: case SXREF:
                diag("undefined external: %s in %s", s->name, TNAME);
                s->type = SDATA;
                // Fall through
            case SDATA: case SBSS: case SDATA1:
                /*s: [[aclass()]] in [[D_ADDR]] case, [[SDATA]] case, if dlm */
                if(dlm) {
                    instoffset = s->value + a->offset + INITDAT;
                    return C_LCON;
                }
                /*e: [[aclass()]] in [[D_ADDR]] case, [[SDATA]] case, if dlm */
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
        /*x: [[aclass()]] [[D_ADDR]] case, switch symkind cases */
        case N_LOCAL:
            instoffset = autosize + a->offset;
            goto aconsize;
        /*x: [[aclass()]] [[D_ADDR]] case, switch symkind cases */
        case N_PARAM:
            instoffset = autosize + a->offset + 4L;
            goto aconsize;
        /*x: [[aclass()]] [[D_ADDR]] case, switch symkind cases */
        aconsize:
            return immrot(instoffset)? C_RACON : C_LACON;
        /*e: [[aclass()]] [[D_ADDR]] case, switch symkind cases */
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
/*e: function [[aclass]](arm) */

/*s: function [[oplook]](arm) */
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
/*e: function [[oplook]](arm) */

/*e: linkers/5l/span.c */
