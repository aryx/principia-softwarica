/*s: linkers/5l/codegen.c */
#include	"l.h"
#include	"m.h"

/*s: function [[oprrr]](arm) */
long
oprrr(int a, int sc)
{
    long o;

    // bits 28 to 32
    o = (sc&C_SCOND) << 28;
    /*s: [[oprrr()]] sign bit handling */
    if(sc & C_SBIT)
        o |= 1 << 20;
    /*e: [[oprrr()]] sign bit handling */
    /*s: [[oprrr()]] sanity check sc */
    if(sc & (C_PBIT|C_WBIT))
        diag(".P/.W on dp instruction");
    /*e: [[oprrr()]] sanity check sc */

    switch(a) {

    // bits 21 to 24 (and sometimes bit 20)
    case AAND:	return o | (0x0<<21);
    case AEOR:	return o | (0x1<<21);
    case ASUB:	return o | (0x2<<21);
    case ARSB:	return o | (0x3<<21);
    case AADD:	return o | (0x4<<21);
    case AADC:	return o | (0x5<<21);
    case ASBC:	return o | (0x6<<21);
    case ARSC:	return o | (0x7<<21);

    case ATST:	return o | (0x8<<21) | (1<<20);
    case ATEQ:	return o | (0x9<<21) | (1<<20);
    case ACMP:	return o | (0xa<<21) | (1<<20);
    case ACMN:	return o | (0xb<<21) | (1<<20);

    case AORR:	return o | (0xc<<21);
    case AMOVW:	return o | (0xd<<21); // MOV
    case ABIC:	return o | (0xe<<21);
    case AMVN:	return o | (0xf<<21); // MVN

    // bits 20 to 27
    /*s: [[oprrr()]] switch cases */
    //                       MOV      shift op
    case ASLL:	return o | (0xd<<21) | (0<<5);
    case ASRL:	return o | (0xd<<21) | (1<<5);
    case ASRA:	return o | (0xd<<21) | (2<<5);
    /*x: [[oprrr()]] switch cases */
    case AMULU:
    case AMUL:	return o | (0x0<<21) | (0x9<<4);
    /*x: [[oprrr()]] switch cases */
    case ASWI:	return o | (0xf<<24);
    /*x: [[oprrr()]] switch cases */
    /* old arm 7500 fp using coprocessor 1 (1<<8) */
    case AADDD:	return o | (0xe<<24) | (0x0<<20) | (1<<8) | (1<<7);
    case AADDF:	return o | (0xe<<24) | (0x0<<20) | (1<<8);
    case AMULD:	return o | (0xe<<24) | (0x1<<20) | (1<<8) | (1<<7);
    case AMULF:	return o | (0xe<<24) | (0x1<<20) | (1<<8);
    case ASUBD:	return o | (0xe<<24) | (0x2<<20) | (1<<8) | (1<<7);
    case ASUBF:	return o | (0xe<<24) | (0x2<<20) | (1<<8);
    case ADIVD:	return o | (0xe<<24) | (0x4<<20) | (1<<8) | (1<<7);
    case ADIVF:	return o | (0xe<<24) | (0x4<<20) | (1<<8);
    /* arguably, ACMPF should expand to RNDF, CMPD */
    case ACMPD:
    case ACMPF:	return o | (0xe<<24) | (0x9<<20) | (0xF<<12) | (1<<8) | (1<<4);	

    case AMOVF:
    case AMOVDF:	return o | (0xe<<24) | (0x0<<20) | (1<<15) | (1<<8);
    case AMOVD:
    case AMOVFD:	return o | (0xe<<24) | (0x0<<20) | (1<<15) | (1<<8) | (1<<7);

    case AMOVWF:	return o | (0xe<<24) | (0<<20) | (1<<8) | (1<<4);
    case AMOVWD:	return o | (0xe<<24) | (0<<20) | (1<<8) | (1<<4) | (1<<7);
    case AMOVFW:	return o | (0xe<<24) | (1<<20) | (1<<8) | (1<<4);
    case AMOVDW:	return o | (0xe<<24) | (1<<20) | (1<<8) | (1<<4) | (1<<7);
    /*x: [[oprrr()]] switch cases */
    case AMULA:	return o | (0x1<<21) | (0x9<<4);
    case AMULLU:	return o | (0x4<<21) | (0x9<<4);
    case AMULL:		return o | (0x6<<21) | (0x9<<4);
    case AMULALU:	return o | (0x5<<21) | (0x9<<4);
    case AMULAL:	return o | (0x7<<21) | (0x9<<4);
    /*e: [[oprrr()]] switch cases */
    }
    diag("bad rrr %d", a);
    prasm(curp);
    return 0;
}
/*e: function [[oprrr]](arm) */

/*s: function [[opvfprrr]](arm) */
long
opvfprrr(int a, int sc)
{
    long o;

    o = (sc & C_SCOND) << 28;
    if(sc & (C_SBIT|C_PBIT|C_WBIT))
        diag(".S/.P/.W on vfp instruction");
    o |= 0xe<<24;
    switch(a) {
    case AMOVWD:	return o | 0xb<<8 | 0xb<<20 | 1<<6 | 0x8<<16 | 1<<7;
    case AMOVWF:	return o | 0xa<<8 | 0xb<<20 | 1<<6 | 0x8<<16 | 1<<7;
    case AMOVDW:	return o | 0xb<<8 | 0xb<<20 | 1<<6 | 0xD<<16 | 1<<7;
    case AMOVFW:	return o | 0xa<<8 | 0xb<<20 | 1<<6 | 0xD<<16 | 1<<7;
    case AMOVFD:	return o | 0xa<<8 | 0xb<<20 | 1<<6 | 0x7<<16 | 1<<7;
    case AMOVDF:	return o | 0xb<<8 | 0xb<<20 | 1<<6 | 0x7<<16 | 1<<7;
    case AMOVF:	return o | 0xa<<8 | 0xb<<20 | 1<<6 | 0x0<<16 | 0<<7;
    case AMOVD:	return o | 0xb<<8 | 0xb<<20 | 1<<6 | 0x0<<16 | 0<<7;
    case ACMPF:	return o | 0xa<<8 | 0xb<<20 | 1<<6 | 0x4<<16 | 0<<7;
    case ACMPD:	return o | 0xb<<8 | 0xb<<20 | 1<<6 | 0x4<<16 | 0<<7;
    case AADDF:	return o | 0xa<<8 | 0x3<<20;
    case AADDD:	return o | 0xb<<8 | 0x3<<20;
    case ASUBF:	return o | 0xa<<8 | 0x3<<20 | 1<<6;
    case ASUBD:	return o | 0xb<<8 | 0x3<<20 | 1<<6;
    case AMULF:	return o | 0xa<<8 | 0x2<<20;
    case AMULD:	return o | 0xb<<8 | 0x2<<20;
    case ADIVF:	return o | 0xa<<8 | 0x8<<20;
    case ADIVD:	return o | 0xb<<8 | 0x8<<20;
    }
    diag("bad vfp rrr %d", a);
    prasm(curp);
    return 0;
}
/*e: function [[opvfprrr]](arm) */

/*s: function [[opbra]](arm) */
long
opbra(int a, int sc)
{

    /*s: [[opbra()]] sanity check sc */
    if(sc & (C_SBIT|C_PBIT|C_WBIT))
        diag(".S/.P/.W on bra instruction");
    /*e: [[opbra()]] sanity check sc */
    sc &= C_SCOND;
    if(a == ABL)
        return (sc<<28)|(0x5<<25)|(0x1<<24);
    /*s: [[opbra()]] sanity check sc if not ABL */
    if(sc != 0xe)
        diag("redundant .EQ/.NE/... on B/BEQ/BNE/...  instruction");
    /*e: [[opbra()]] sanity check sc if not ABL */

    switch(a) {
    // manual setting of the conditional execution
    // bits 28 to 32
    case ABEQ:	return (0x0<<28)|(0x5<<25);
    case ABNE:	return (0x1<<28)|(0x5<<25);
    case ABHS:	return (0x2<<28)|(0x5<<25);
    case ABLO:	return (0x3<<28)|(0x5<<25);
    case ABMI:	return (0x4<<28)|(0x5<<25);
    case ABPL:	return (0x5<<28)|(0x5<<25);
    case ABVS:	return (0x6<<28)|(0x5<<25);
    case ABVC:	return (0x7<<28)|(0x5<<25);
    case ABHI:	return (0x8<<28)|(0x5<<25);
    case ABLS:	return (0x9<<28)|(0x5<<25);
    case ABGE:	return (0xa<<28)|(0x5<<25);
    case ABLT:	return (0xb<<28)|(0x5<<25);
    case ABGT:	return (0xc<<28)|(0x5<<25);
    case ABLE:	return (0xd<<28)|(0x5<<25);
    case AB:	return (0xe<<28)|(0x5<<25);
    }
    diag("bad bra %A", a);
    prasm(curp);
    return 0;
}
/*e: function [[opbra]](arm) */

/*s: function [[olr]](arm) */
long
olr(int a, int sc, long v, int b, int rt)
{
    long o;

    o = (sc&C_SCOND) << 28;
    /*s: [[olr()]] sanity check sc */
    if(sc & C_SBIT)
        diag(".S on LDR/STR instruction");
    if(sc & C_UBIT)
        diag(".U on LDR/STR instruction");
    /*e: [[olr()]] sanity check sc */

    /*s: [[olr()]] set special bits */
    if(!(sc & C_PBIT))
        o |= 1 << 24; // pre (not post)
    if(sc & C_WBIT)
        o |= 1 << 21; // write back
    /*e: [[olr()]] set special bits */
    o |= (0x1<<26) | // memory instructions class
         (1<<20);    // LDR
    if(v >= 0) {
        o |= 1 << 23; // Up bit, positive offset
    } else {
        // bit 23 unset, Down, negative offset
        v = -v;
    }
    /*s: [[olr()]] sanity check offset [[v]] */
    if(v >= (1<<12))
        diag("literal span too large: %ld (R%d)\n%P", v, b, curp);
    /*e: [[olr()]] sanity check offset [[v]] */
    switch(a) {
    case AMOVB:
    case AMOVBU: 
        o |= 1<<22; 
        break;
    case AMOVW: 
        break;
    default:
        /*s: [[olr()]] sanity check [[a]] */
        diag("expect move operation, not: %P", curp);
        /*e: [[olr()]] sanity check [[a]] */
    }        
    o |= (b<<16) | (rt<<12) | v;
    return o;
}
/*e: function [[olr]](arm) */

/*s: function [[olhr]](arm) */
long
olhr(long v, int b, int r, int sc)
{
    long o;

    o = (sc & C_SCOND) << 28;
    if(sc & C_SBIT)
        diag(".S on LDRH/STRH instruction");

    if(!(sc & C_PBIT))
        o |= 1 << 24;
    if(sc & C_WBIT)
        o |= 1 << 21;
    o |= (1<<23) | (1<<20) | (0xb<<4);
    if(v < 0) {
        v = -v;
        o ^= 1 << 23;
    }
    if(v >= (1<<8))
        diag("literal span too large: %ld (R%d)\n%P", v, b, curp);
    o |= (v&0xf)|((v>>4)<<8)|(1<<22);
    o |= b << 16;
    o |= r << 12;
    return o;
}
/*e: function [[olhr]](arm) */

/*s: function [[osr]](arm) */
long
osr(int a, int sc, int r, long v, int b)
{

    return olr(a, sc, v, b, r) ^ (1<<20); // STR, unset (via xor) bit 20
}
/*e: function [[osr]](arm) */

/*s: function [[oshr]](arm) */
long
oshr(int r, long v, int b, int sc)
{
    long o;

    o = olhr(v, b, r, sc) ^ (1<<20);
    return o;
}
/*e: function [[oshr]](arm) */
    

/*s: function [[olrr]](arm) */
long
olrr(int a, int sc, int i, int b, int r)
{

    return olr(a, sc, i, b, r) | (1<<25); // Rm not immediate offset
}
/*e: function [[olrr]](arm) */

/*s: function [[olhrr]](arm) */
long
olhrr(int i, int b, int r, int sc)
{
    return olhr(i, b, r, sc) ^ (1<<22);
}
/*e: function [[olhrr]](arm) */

/*s: function [[osrr]](arm) */
long
osrr(int a, int sc, int r, int i, int b)
{

    return olrr(a, sc, i, b, r) ^ (1<<20); // STR
}
/*e: function [[osrr]](arm) */

/*s: function [[oshrr]](arm) */
long
oshrr(int r, int i, int b, int sc)
{
    return olhr(i, b, r, sc) ^ ((1<<22) | (1<<20));
}
/*e: function [[oshrr]](arm) */


/*s: function [[ovfpmem]](arm) */
long
ovfpmem(int a, int r, long v, int b, int sc, Prog *p)
{
    long o;

    o = (sc & C_SCOND) << 28;
    if(sc & (C_SBIT|C_PBIT|C_WBIT))
        diag(".S/.P/.W on VLDR/VSTR instruction");
    o |= 0xd<<24 | (1<<23);
    if(v < 0) {
        v = -v;
        o ^= 1 << 23;
    }
    if(v & 3)
        diag("odd offset for floating point op: %ld\n%P", v, p);
    else if(v >= (1<<10))
        diag("literal span too large: %ld\n%P", v, p);
    o |= (v>>2) & 0xFF;
    o |= b << 16;
    o |= r << 12;
    switch(a) {
    default:
        diag("bad fst %A", a);
    case AMOVD:
        o |= 0xb<<8;
        break;
    case AMOVF:
        o |= 0xa<<8;
        break;
    }
    return o;
}
/*e: function [[ovfpmem]](arm) */

/*s: function [[ofsr]](arm) */
long
ofsr(int a, int r, long v, int b, int sc, Prog *p)
{
    long o;

    if(vfp)
        return ovfpmem(a, r, v, b, sc, p);

    o = (sc & C_SCOND) << 28;
    if(sc & C_SBIT)
        diag(".S on FLDR/FSTR instruction");
    if(!(sc & C_PBIT))
        o |= 1 << 24;
    if(sc & C_WBIT)
        o |= 1 << 21;
    o |= (6<<25) | (1<<24) | (1<<23);
    if(v < 0) {
        v = -v;
        o ^= 1 << 23;
    }
    if(v & 3)
        diag("odd offset for floating point op: %ld\n%P", v, p);
    else if(v >= (1<<10))
        diag("literal span too large: %ld\n%P", v, p);
    o |= (v>>2) & 0xFF;
    o |= b << 16;
    o |= r << 12;
    o |= 1 << 8;

    switch(a) {
    default:
        diag("bad fst %A", a);
    case AMOVD:
        o |= 1<<15;
    case AMOVF:
        break;
    }
    return o;
}
/*e: function [[ofsr]](arm) */

/*s: function [[omvl]](arm) */
long
omvl(Prog *p, Adr *a, int dr)
{	
    long v, o1;

    /*s: [[omvl()]] when [[C_LCON]] case */
    if(p->cond) {
        // C_LCON case with Pool
        v = p->cond->pc - p->pc - 8;
        // =~ LDR v(R15), R11
        o1 = olr(AMOVW, p->scond&C_SCOND, v, REGPC, dr);
    }
    /*e: [[omvl()]] when [[C_LCON]] case */
    else {
        // C_NCON case
        aclass(a);
        v = immrot(~instoffset);
        /*s: [[omvl()]] sanity check v */
        if(v == 0) {
            diag("missing literal");
            prasm(p);
            return 0;
        }
        /*e: [[omvl()]] sanity check v */
        o1 = oprrr(AMVN, p->scond&C_SCOND);
        o1 |= (dr<<12) | v;
    }
    return o1;
}
/*e: function [[omvl]](arm) */


/*s: function [[asmout]](arm) */
/// main -> asmb -> for p { <> }
void
asmout(Prog *p, Optab *o)
{
    // ARM 32 bits instructions, set in the switch
    long o1, o2, o3, o4, o5, o6;
    /*s: [[asmout()]] other locals */
    int rf; // register ``from''
    int rt; // register ``to''
    int r;  // middle register
    Sym *s;
    /*x: [[asmout()]] other locals */
    int c;
    /*x: [[asmout()]] other locals */
    int rt2;
    /*x: [[asmout()]] other locals */
    // pc (real)
    long v;
    /*e: [[asmout()]] other locals */

    o1 = o2 = o3 = o4 = o5 = o6 = 0;

    // first switch, action id dispatch, set o1, o2, ...
    switch(o->type) {
    /*s: [[asmout()]] switch on type cases */
    case 0:		/* pseudo ops */
        break;
    /*x: [[asmout()]] switch on type cases */
    case 11:	/* word */
        c = aclass(&p->to);
        /*s: [[asmout()]] in AWORD case, when dlm */
        switch(c) {
        case C_LCON:
            if(!dlm)
                break;
            if(p->to.symkind != N_EXTERN && p->to.symkind != N_INTERN)
                break;
            // Fallthrough
        case C_ADDR:
            if(p->to.sym->type == SUNDEF)
                ckoff(p->to.sym, p->to.offset);
            dynreloc(p->to.sym, p->pc, 1);
        }
        /*e: [[asmout()]] in AWORD case, when dlm */
        o1 = instoffset;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 1:		/* op R,[R],R */
        o1 = oprrr(p->as, p->scond);
        rf = p->from.reg;
        rt = p->to.reg;
        r = p->reg;
        /*s: [[asmout()]] adjust [[r]] and [[rt]] */
        if(p->to.type == D_NONE)
            rt = 0;
        if(p->as == AMOVW || p->as == AMVN)
            r = 0;
        else 
          /*s: [[asmout()]] adjust [[r]] */
          if(r == R_NONE) // ADD FROM, TO ==> ADD FROM, TO, TO
              r = rt;
          /*e: [[asmout()]] adjust [[r]] */
        /*e: [[asmout()]] adjust [[r]] and [[rt]] */
        o1 |= (r<<16) | (rt<<12) | rf;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 2:		/* op $I,[R],R */
        aclass(&p->from);
        o1 = oprrr(p->as, p->scond);
        o1 |= immrot(instoffset); // set also bit 25 for Immediate
        rt = p->to.reg;
        r = p->reg;
        /*s: [[asmout()]] adjust [[r]] and [[rt]] */
        if(p->to.type == D_NONE)
            rt = 0;
        if(p->as == AMOVW || p->as == AMVN)
            r = 0;
        else 
          /*s: [[asmout()]] adjust [[r]] */
          if(r == R_NONE) // ADD FROM, TO ==> ADD FROM, TO, TO
              r = rt;
          /*e: [[asmout()]] adjust [[r]] */
        /*e: [[asmout()]] adjust [[r]] and [[rt]] */
        o1 |= (r<<16) | (rt<<12);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 3:		/* op R<<[IR],[R],R */
    mov:
        aclass(&p->from);
        o1 = oprrr(p->as, p->scond);
        o1 |= p->from.offset; // set in 5a, complex bit layout
        rt = p->to.reg;
        r = p->reg;
        /*s: [[asmout()]] adjust [[r]] and [[rt]] */
        if(p->to.type == D_NONE)
            rt = 0;
        if(p->as == AMOVW || p->as == AMVN)
            r = 0;
        else 
          /*s: [[asmout()]] adjust [[r]] */
          if(r == R_NONE) // ADD FROM, TO ==> ADD FROM, TO, TO
              r = rt;
          /*e: [[asmout()]] adjust [[r]] */
        /*e: [[asmout()]] adjust [[r]] and [[rt]] */
        o1 |= (r<<16) | (rt<<12);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 8:		/* sll $c,[R],R -> mov (R<<$c),R */
        aclass(&p->from);
        o1 = oprrr(p->as, p->scond);
        rt = p->to.reg;
        r = p->reg;
        /*s: [[asmout()]] adjust [[r]] */
        if(r == R_NONE) // ADD FROM, TO ==> ADD FROM, TO, TO
            r = rt;
        /*e: [[asmout()]] adjust [[r]] */
        o1 |= (rt<<12) | ((instoffset&31) << 7) | r;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 9:		/* sll R,[R],R -> mov (R<<R),R */
        o1 = oprrr(p->as, p->scond);
        rf = p->from.reg;
        rt = p->to.reg;
        r = p->reg;
        /*s: [[asmout()]] adjust [[r]] */
        if(r == R_NONE) // ADD FROM, TO ==> ADD FROM, TO, TO
            r = rt;
        /*e: [[asmout()]] adjust [[r]] */
        o1 |= (rt<<12) | (rf<<8) | (1<<4) | r ;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 14:	/* movb/movbu/movh/movhu R,R */
        o1 = oprrr(ASLL, p->scond);

        if(p->as == AMOVBU || p->as == AMOVHU)
            o2 = oprrr(ASRL, p->scond);
        else
            o2 = oprrr(ASRA, p->scond);

        rf = p->from.reg;
        rt = p->to.reg;
        if(p->as == AMOVB || p->as == AMOVBU) {
            o1 |= (rt<<12) | (24<<7) | rf;
            o2 |= (rt<<12) | (24<<7) | rt;
        } else {
            o1 |= (rt<<12) | (16<<7) | rf;
            o2 |= (rt<<12) | (16<<7) | rt;
        }
        break;
    /*x: [[asmout()]] switch on type cases */
    case 58:	/* movbu R,R -> AND $0xff, R, R */
        o1 = oprrr(AAND, p->scond);
        o1 |= immrot(0xff);
        rt = p->to.reg;
        r = p->from.reg;
        if(p->to.type == D_NONE)
            rt = 0;
        /*s: [[asmout()]] adjust [[r]] */
        if(r == R_NONE) // ADD FROM, TO ==> ADD FROM, TO, TO
            r = rt;
        /*e: [[asmout()]] adjust [[r]] */
        o1 |= (r<<16) | (rt<<12);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 15:	/* mul r,[r,]r */
        o1 = oprrr(p->as, p->scond);
        rf = p->from.reg;
        rt = p->to.reg;
        r = p->reg;
        /*s: [[asmout()]] adjust [[r]] */
        if(r == R_NONE) // ADD FROM, TO ==> ADD FROM, TO, TO
            r = rt;
        /*e: [[asmout()]] adjust [[r]] */
        /*s: [[asmout()]] adjust registers when mul */
        if(rt == r) {
            r = rf;
            rf = rt;
        }
        /*e: [[asmout()]] adjust registers when mul */
        o1 |= (rt<<16) | (rf<<8) | r;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 13:	/* op $lcon, [R], R */
        o1 = omvl(p, &p->from, REGTMP);
        /*s: [[asmout()]] sanity check o1 */
        if(!o1)
            break;
        /*e: [[asmout()]] sanity check o1 */

        o2 = oprrr(p->as, p->scond);
        rf = REGTMP;
        rt = p->to.reg;
        r = p->reg;
        if(p->as == AMOVW || p->as == AMVN) // can be AMOVW??
            r = 0;
        else 
          /*s: [[asmout()]] adjust [[r]] */
          if(r == R_NONE) // ADD FROM, TO ==> ADD FROM, TO, TO
              r = rt;
          /*e: [[asmout()]] adjust [[r]] */
        o2 |= (r<<16) | rf;
        if(p->to.type != D_NONE)
            o2 |= rt << 12;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 12:	/* movw $lcon, reg */
        o1 = omvl(p, &p->from, p->to.reg);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 5:		/* bra s */
        /*s: [[asmout()]] BRA case, if undefined target */
        if(p->cond == UP) {
            s = p->to.sym;
            if(s->type != SUNDEF)
                diag("bad branch sym type");
            v = (ulong)s->value >> (Roffset-2);
            dynreloc(s, p->pc, 0);
        }
        /*e: [[asmout()]] BRA case, if undefined target */
        else 
          if(p->cond != P)
            v = (p->cond->pc - p->pc) - 8;
          else 
            v = -8; // warning?
        o1 = opbra(p->as, p->scond);
        o1 |= (v >> 2) & 0xffffff;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 6:		/* b O(R) -> add $O,R,PC */
        aclass(&p->to);
        o1 = oprrr(AADD, p->scond);
        o1 |= immrot(instoffset);
        r = p->to.reg;
        rt = REGPC; 
        o1 |= (r<<16) | (rt<<12);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 7:		/* bl Offset(R) -> ADD $0,PC,LINK; add $Offset,R,PC */
        aclass(&p->to);
        o1 = oprrr(AADD, p->scond);
        rt = REGLINK;
        r = REGPC;
        o1 |= (r<<16) | (rt<<12) | immrot(0);

        o2 = oprrr(AADD, p->scond);
        r = p->to.reg;
        rt = REGPC;
        o2 |= (r<<16) | (rt<<12) | immrot(instoffset);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 21:	/* mov/movbu O(R),R */
        aclass(&p->from);
        rt = p->to.reg;
        r = p->from.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o1 = olr(p->as, p->scond, instoffset, r, rt);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 31:	/* mov/movbu L(R),R */
        o1 = omvl(p, &p->from, REGTMP);
        /*s: [[asmout()]] sanity check o1 */
        if(!o1)
            break;
        /*e: [[asmout()]] sanity check o1 */

        rt = p->to.reg;
        r = p->from.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o2 = olrr(p->as, p->scond, REGTMP, r, rt);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 20:	/* mov/movb/movbu R,O(R) */
        aclass(&p->to);
        rf = p->from.reg;
        r = p->to.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o1 = osr(p->as, p->scond, rf, instoffset, r);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 30:	/* mov/movb/movbu R,L(R) */
        o1 = omvl(p, &p->to, REGTMP);
        /*s: [[asmout()]] sanity check o1 */
        if(!o1)
            break;
        /*e: [[asmout()]] sanity check o1 */

        rf = p->from.reg;
        r = p->to.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o2 = osrr(p->as, p->scond, rf, REGTMP, r);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 40:	/* swp oreg,reg,reg */
        aclass(&p->from);
        /*s: [[asmout()]] sanity check instoffset for SWP */
        if(instoffset != 0)
            diag("offset must be zero in SWP");
        /*e: [[asmout()]] sanity check instoffset for SWP */
        o1 = (0x2<<23) | (0x9<<4); // SWP
        if(p->as == ASWPBU)
            o1 |= 1 << 22;
        rf = p->from.reg;
        rt = p->to.reg;
        r = p->reg;
        o1 |= ((p->scond & C_SCOND) << 28) | (rf<<16) | (rt<<12) | r;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 4:		/* add $I,[R],R */
        aclass(&p->from);
        o1 = oprrr(AADD, p->scond);
        rt = p->to.reg;
        r = p->from.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o1 |= (r<<16) | (rt<<12) | immrot(instoffset);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 34:	/* mov $lacon,R -> LDR x(R15), R11; ADD R11, R13, R */
        o1 = omvl(p, &p->from, REGTMP);
        /*s: [[asmout()]] sanity check o1 */
        if(!o1)
            break;
        /*e: [[asmout()]] sanity check o1 */

        o2 = oprrr(AADD, p->scond);
        rf = REGTMP;
        rt = p->to.reg;
        r = p->from.reg; 
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o2 |= (r<<16) | (rt<<12) | rf;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 22:	/* movb/movh/movhu O(R),R -> lr,shl,shr */
        aclass(&p->from);
        rt = p->to.reg;
        r = p->from.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o1 = olr(AMOVW, p->scond, instoffset, r, rt);

        o2 = oprrr(ASLL, p->scond);
        if(p->as == AMOVHU)
            o3 = oprrr(ASRL, p->scond);
        else
            o3 = oprrr(ASRA, p->scond);

        if(p->as == AMOVB) {
            o2 |= (rt<<12) | (24<<7) | rt;
            o3 |= (rt<<12) | (24<<7) | rt;
        } else {
            o2 |= (rt<<12) | (16<<7) | rt;
            o3 |= (rt<<12) | (16<<7) | rt;
        }
        break;
    /*x: [[asmout()]] switch on type cases */
    case 32:	/* movh/movb L(R),R */
        o1 = omvl(p, &p->from, REGTMP);
        /*s: [[asmout()]] sanity check o1 */
        if(!o1)
            break;
        /*e: [[asmout()]] sanity check o1 */

        rt = p->to.reg;
        r = p->from.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o2 = olrr(p->as, p->scond, REGTMP,r, rt);

        o3 = oprrr(ASLL, p->scond);
        if(p->as == AMOVHU)
            o4 = oprrr(ASRL, p->scond);
        else
            o4 = oprrr(ASRA, p->scond);

        if(p->as == AMOVB) {
            o3 |= (rt<<12) | (24<<7) | rt;
            o4 |= (rt<<12) | (24<<7) | rt;
        } else {
            o3 |= (rt<<12) | (16<<7) | rt;
            o4 |= (rt<<12) | (16<<7) | rt;
        }
        break;
    /*x: [[asmout()]] switch on type cases */
    case 23:	/* movh/movhu R,O(R) -> sb,sb */
        aclass(&p->to);
        rf = p->from.reg;
        r = p->to.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o1 = osr(AMOVBU, p->scond, rf, instoffset, r);

        o2 = oprrr(ASRL, p->scond);
        rt = REGTMP;
        o2 |= (rt<<12) | (8<<7) | rf;

        o3 = osr(AMOVBU, p->scond, REGTMP, instoffset+1, r);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 33:	/* movh/movhu R,L(R) -> sb, sb */
        o1 = omvl(p, &p->to, REGTMP);
        /*s: [[asmout()]] sanity check o1 */
        if(!o1)
            break;
        /*e: [[asmout()]] sanity check o1 */

        rf = p->from.reg;
        r = p->to.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o2 = osrr(AMOVBU, p->scond, rf, REGTMP, r);

        o3 = oprrr(ASRL, p->scond);
        o3 |= (rf<<12) | (8<<7) | rf;
        o3 |= (1<<6);	/* ROR 8 */

        o4 = oprrr(AADD, p->scond);
        o4 |= (REGTMP<<16) | (REGTMP<<12);
        o4 |= immrot(1);

        o5 = osrr(AMOVBU, p->scond, rf, REGTMP,r);

        // restore rf
        o6 = oprrr(ASRL, p->scond);
        o6 |= (rf<<12) | (24<<7) | rf;
        o6 |= (1<<6);	/* ROL 8 */

        break;
    /*x: [[asmout()]] switch on type cases */
    case 10:	/* swi [$con] */
        o1 = oprrr(p->as, p->scond);
        if(p->to.type != D_NONE) {
            aclass(&p->to);
            o1 |= instoffset & 0xffffff;
        }
        break;
    /*x: [[asmout()]] switch on type cases */
    case 41:	/* rfe -> movm.s.w.u 0(r13),[r15] */
        o1 = 0xe8fd8000;
        break;
    /*x: [[asmout()]] switch on type cases */
    /* reloc ops */
    case 64:	/* mov/movb/movbu R,addr */
        o1 = omvl(p, &p->to, REGTMP);
        if(!o1)
            break;
        o2 = osr(p->as, p->scond, p->from.reg, 0, REGTMP);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 65:	/* mov/movbu addr,R */
    case 66:	/* movh/movhu/movb addr,R */
        o1 = omvl(p, &p->from, REGTMP);
        if(!o1)
            break;
        o2 = olr(p->as, p->scond, 0, REGTMP, p->to.reg);
        if(o->type == 65)
            break;

        o3 = oprrr(ASLL, p->scond);

        if(p->as == AMOVBU || p->as == AMOVHU)
            o4 = oprrr(ASRL, p->scond);
        else
            o4 = oprrr(ASRA, p->scond);

        r = p->to.reg;
        o3 |= (r)|(r<<12);
        o4 |= (r)|(r<<12);
        if(p->as == AMOVB || p->as == AMOVBU) {
            o3 |= (24<<7);
            o4 |= (24<<7);
        } else {
            o3 |= (16<<7);
            o4 |= (16<<7);
        }
        break;
    /*x: [[asmout()]] switch on type cases */
    case 67:	/* movh/movhu R,addr -> sb, sb */
        o1 = omvl(p, &p->to, REGTMP);
        if(!o1)
            break;
        o2 = osr(p->as, p->scond, p->from.reg, 0, REGTMP);

        o3 = oprrr(ASRL, p->scond);
        o3 |= (8<<7)|(p->from.reg)|(p->from.reg<<12);
        o3 |= (1<<6);	/* ROR 8 */

        o4 = oprrr(AADD, p->scond);
        o4 |= (REGTMP << 12) | (REGTMP << 16);
        o4 |= immrot(1);

        o5 = osr(p->as, p->scond, p->from.reg, 0, REGTMP);

        o6 = oprrr(ASRL, p->scond);
        o6 |= (24<<7)|(p->from.reg)|(p->from.reg<<12);
        o6 |= (1<<6);	/* ROL 8 */
        break;
    /*x: [[asmout()]] switch on type cases */
    case 50:	/* floating point store */
        v = regoff(&p->to);
        r = p->to.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o1 = ofsr(p->as, p->from.reg, v, r, p->scond, p);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 51:	/* floating point load */
        v = regoff(&p->from);
        r = p->from.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o1 = ofsr(p->as, p->to.reg, v, r, p->scond, p) | (1<<20);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 52:	/* floating point store, long offset UGLY */
        o1 = omvl(p, &p->to, REGTMP);
        if(!o1)
            break;
        r = p->to.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o2 = oprrr(AADD, p->scond) | (REGTMP << 12) | (REGTMP << 16) | r;
        o3 = ofsr(p->as, p->from.reg, 0, REGTMP, p->scond, p);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 53:	/* floating point load, long offset UGLY */
        o1 = omvl(p, &p->from, REGTMP);
        if(!o1)
            break;
        r = p->from.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o2 = oprrr(AADD, p->scond) | (REGTMP << 12) | (REGTMP << 16) | r;
        o3 = ofsr(p->as, p->to.reg, 0, REGTMP, p->scond, p) | (1<<20);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 54:	/* floating point arith */
        o1 = oprrr(p->as, p->scond);
        if(p->from.type == D_FCONST) {
            rf = chipfloat(p->from.ieee);
            if(rf < 0){
                diag("invalid floating-point immediate\n%P", p);
                rf = 0;
            }
            rf |= (1<<3);
        } else
            rf = p->from.reg;
        rt = p->to.reg;
        r = p->reg;
        if(p->to.type == D_NONE)
            rt = 0;	/* CMP[FD] */
        else if(o1 & (1<<15))
            r = 0;	/* monadic */
        else 
          /*s: [[asmout()]] adjust [[r]] */
          if(r == R_NONE) // ADD FROM, TO ==> ADD FROM, TO, TO
              r = rt;
          /*e: [[asmout()]] adjust [[r]] */
        o1 |= rf | (r<<16) | (rt<<12);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 55:	/* floating point fix and float */
        o1 = oprrr(p->as, p->scond);
        rf = p->from.reg;
        rt = p->to.reg;
        if(p->to.type == D_NONE){
            rt = 0;
            diag("to.type==D_NONE (asm/fp)");
        }
        if(p->from.type == D_REG)
            o1 |= (rf<<12) | (rt<<16);
        else
            o1 |= rf | (rt<<12);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 68:	/* floating point store -> ADDR */
        o1 = omvl(p, &p->to, REGTMP);
        if(!o1)
            break;
        o2 = ofsr(p->as, p->from.reg, 0, REGTMP, p->scond, p);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 69:	/* floating point load <- ADDR */
        o1 = omvl(p, &p->from, REGTMP);
        if(!o1)
            break;
        o2 = ofsr(p->as, p->to.reg, 0, REGTMP, p->scond, p) | (1<<20);
        break;
    /*x: [[asmout()]] switch on type cases */
    /* old arm 7500 fp using coprocessor 1 (1<<8) */
    case 56:	/* move to FP[CS]R */
        o1 = ((p->scond & C_SCOND) << 28) | (0xe << 24) | (1<<8) | (1<<4);
        o1 |= ((p->to.reg+1)<<21) | (p->from.reg << 12);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 57:	/* move from FP[CS]R */
        o1 = ((p->scond & C_SCOND) << 28) | (0xe << 24) | (1<<8) | (1<<4);
        o1 |= ((p->from.reg+1)<<21) | (p->to.reg<<12) | (1<<20);
        break;
    /*x: [[asmout()]] switch on type cases */
    /* VFP ops: */
    case 74:	/* vfp floating point arith */
        o1 = opvfprrr(p->as, p->scond);
        rf = p->from.reg;
        if(p->from.type == D_FCONST) {
            diag("invalid floating-point immediate\n%P", p);
            rf = 0;
        }
        rt = p->to.reg;
        r = p->reg;
        /*s: [[asmout()]] adjust [[r]] */
        if(r == R_NONE) // ADD FROM, TO ==> ADD FROM, TO, TO
            r = rt;
        /*e: [[asmout()]] adjust [[r]] */
        o1 |= rt<<12;
        if(((o1>>20)&0xf) == 0xb)
            o1 |= rf<<0;
        else
            o1 |= r<<16 | rf<<0;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 75:	/* vfp floating point compare */
        o1 = opvfprrr(p->as, p->scond);
        rf = p->from.reg;
        if(p->from.type == D_FCONST) {
            if(p->from.ieee->h != 0 || p->from.ieee->l != 0)
                diag("invalid floating-point immediate\n%P", p);
            o1 |= 1<<16;
            rf = 0;
        }
        rt = p->reg;
        o1 |= rt<<12 | rf<<0;
        o2 = 0x0ef1fa10;	/* MRS APSR_nzcv, FPSCR */
        o2 |= (p->scond & C_SCOND) << 28;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 76:	/* vfp floating point fix and float */
        o1 = opvfprrr(p->as, p->scond);
        rf = p->from.reg;
        rt = p->to.reg;
        if(p->from.type == D_REG) {
            o2 = o1 | rt<<12 | rt<<0;
            o1 = 0x0e000a10;	/* VMOV F,R */
            o1 |= (p->scond & C_SCOND) << 28 | rt<<16 | rf<<12;
        } else {
            o1 |= FREGTMP<<12 | rf<<0;
            o2 = 0x0e100a10;	/* VMOV R,F */
            o2 |= (p->scond & C_SCOND) << 28 | FREGTMP<<16 | rt<<12;
        }
        break;
    /*x: [[asmout()]] switch on type cases */
    case 16:	/* div r,[r,]r */
        o1 = 0xf << 28;
        o2 = 0;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 17:
        o1 = oprrr(p->as, p->scond);
        rf = p->from.reg;
        rt = p->to.reg;
        rt2 = p->to.offset;
        r = p->reg;
        o1 |= (rf<<8) | r | (rt<<16) | (rt2<<12);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 38:	/* movm $con,oreg -> stm */
        o1 = (0x4 << 25);
        o1 |= p->from.offset & 0xffff;
        o1 |= p->to.reg << 16;
        aclass(&p->to);
        goto movm;
    /*x: [[asmout()]] switch on type cases */
    case 39:	/* movm oreg,$con -> ldm */
        o1 = (0x4 << 25) | (1 << 20);
        o1 |= p->to.offset & 0xffff;
        o1 |= p->from.reg << 16;
        aclass(&p->from);
    movm:
        if(instoffset != 0)
            diag("offset must be zero in MOVM");
        o1 |= (p->scond & C_SCOND) << 28;
        if(p->scond & C_PBIT)
            o1 |= 1 << 24;
        if(p->scond & C_UBIT)
            o1 |= 1 << 23;
        if(p->scond & C_SBIT)
            o1 |= 1 << 22;
        if(p->scond & C_WBIT)
            o1 |= 1 << 21;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 35:	/* mov PSR,R */
        o1 = (2<<23) | (0xf<<16) | (0<<0);
        o1 |= (p->scond & C_SCOND) << 28;
        o1 |= (p->from.reg & 1) << 22;
        o1 |= p->to.reg << 12;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 36:	/* mov R,PSR */
        o1 = (2<<23) | (0x29f<<12) | (0<<4);
        if(p->scond & C_FBIT)
            o1 ^= 0x010 << 12;
        o1 |= (p->scond & C_SCOND) << 28;
        o1 |= (p->to.reg & 1) << 22;
        o1 |= p->from.reg << 0;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 37:	/* mov $con,PSR */
        aclass(&p->from);
        o1 = (2<<23) | (0x29f<<12) | (0<<4);
        if(p->scond & C_FBIT)
            o1 ^= 0x010 << 12;
        o1 |= (p->scond & C_SCOND) << 28;
        o1 |= immrot(instoffset);
        o1 |= (p->to.reg & 1) << 22;
        o1 |= p->from.reg << 0;
        break;
    /*x: [[asmout()]] switch on type cases */
    /* ArmV4 ops: */
    case 70:	/* movh/movhu R,O(R) -> strh */
        aclass(&p->to);
        r = p->to.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o1 = oshr(p->from.reg, instoffset, r, p->scond);
        break;	
    /*x: [[asmout()]] switch on type cases */
    case 71:	/* movb/movh/movhu O(R),R -> ldrsb/ldrsh/ldrh */
        aclass(&p->from);
        r = p->from.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o1 = olhr(instoffset, r, p->to.reg, p->scond);
        if(p->as == AMOVB)
            o1 ^= (1<<5)|(1<<6);
        else if(p->as == AMOVH)
            o1 ^= (1<<6);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 72:	/* movh/movhu R,L(R) -> strh */
        o1 = omvl(p, &p->to, REGTMP);
        if(!o1)
            break;
        r = p->to.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o2 = oshrr(p->from.reg, REGTMP,r, p->scond);
        break;	
    /*x: [[asmout()]] switch on type cases */
    case 73:	/* movb/movh/movhu L(R),R -> ldrsb/ldrsh/ldrh */
        o1 = omvl(p, &p->from, REGTMP);
        if(!o1)
            break;
        r = p->from.reg;
        /*s: [[asmout()]] adjust maybe r to rule param */
        if(r == R_NONE)
            r = o->param;
        /*e: [[asmout()]] adjust maybe r to rule param */
        o2 = olhrr(REGTMP, r, p->to.reg, p->scond);
        if(p->as == AMOVB)
            o2 ^= (1<<5)|(1<<6);
        else if(p->as == AMOVH)
            o2 ^= (1<<6);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 59:	/* movw/bu R<<I(R),R -> ldr indexed */
        if(p->from.reg == R_NONE) {
            if(p->as != AMOVW)
                diag("byte MOV from shifter operand");
            goto mov;
        }
        if(p->from.offset&(1<<4))
            diag("bad shift in LDR");
        o1 = olrr(p->as, p->scond, p->from.offset, p->from.reg, p->to.reg);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 60:	/* movb R(R),R -> ldrsb indexed */
        if(p->from.reg == R_NONE) {
            diag("byte MOV from shifter operand");
            goto mov;
        }
        if(p->from.offset&(~0xf))
            diag("bad shift in LDRSB");
        o1 = olhrr(p->from.offset, p->from.reg, p->to.reg, p->scond);
        o1 ^= (1<<5)|(1<<6);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 61:	/* movw/b/bu R,R<<[IR](R) -> str indexed */
        if(p->to.reg == R_NONE)
            diag("MOV to shifter operand");
        o1 = osrr(p->as, p->scond, p->from.reg, p->to.offset, p->to.reg);
        break;
    /*x: [[asmout()]] switch on type cases */
    case 62:	/* case R -> movw	R<<2(PC),PC */
        o1 = olrr(AMOVW, p->scond, p->from.reg, REGPC, REGPC);
        o1 |= 2<<7;
        break;
    /*x: [[asmout()]] switch on type cases */
    case 63:	/* bcase */
        if(p->cond != P) {
            o1 = p->cond->pc;
            if(dlm)
                dynreloc(S, p->pc, 1);
        }
        break;
    /*e: [[asmout()]] switch on type cases */
    default:
        diag("unknown asm %d", o->type);
        prasm(p);
        break;
    }

    /*s: [[asmout()]] debug */
    if(debug['a'] > 1)
        Bprint(&bso, "%2d ", o->type);
    v = p->pc; // for debugging later
    /*e: [[asmout()]] debug */

    // second switch, output o1, o2, ...
    switch(o->size) {
    /*s: [[asmout()]] switch on size cases */
    case 4:
        /*s: [[asmout()]] when 1 generated instruction, debug */
        if(debug['a'])
            Bprint(&bso, " %.8lux: %.8lux\t%P\n", v, o1, p);
        /*e: [[asmout()]] when 1 generated instruction, debug */
        lputl(o1);
        break;
    case 8:
        /*s: [[asmout()]] when 2 generated instructions, debug */
        if(debug['a'])
            Bprint(&bso, " %.8lux: %.8lux %.8lux%P\n", v, o1, o2, p);
        /*e: [[asmout()]] when 2 generated instructions, debug */
        lputl(o1);
        lputl(o2);
        break;
    case 12:
        /*s: [[asmout()]] when 3 generated instructions, debug */
        if(debug['a'])
            Bprint(&bso, " %.8lux: %.8lux %.8lux %.8lux%P\n", v, o1, o2, o3, p);
        /*e: [[asmout()]] when 3 generated instructions, debug */
        lputl(o1);
        lputl(o2);
        lputl(o3);
        break;
    case 16:
        /*s: [[asmout()]] when 4 generated instructions, debug */
        if(debug['a'])
            Bprint(&bso, " %.8lux: %.8lux %.8lux %.8lux %.8lux%P\n",
                v, o1, o2, o3, o4, p);
        /*e: [[asmout()]] when 4 generated instructions, debug */
        lputl(o1);
        lputl(o2);
        lputl(o3);
        lputl(o4);
        break;
    case 20:
        /*s: [[asmout()]] when 5 generated instructions, debug */
        if(debug['a'])
            Bprint(&bso, " %.8lux: %.8lux %.8lux %.8lux %.8lux %.8lux%P\n",
                v, o1, o2, o3, o4, o5, p);
        /*e: [[asmout()]] when 5 generated instructions, debug */
        lputl(o1);
        lputl(o2);
        lputl(o3);
        lputl(o4);
        lputl(o5);
        break;
    case 24:
        /*s: [[asmout()]] when 6 generated instructions, debug */
        if(debug['a'])
            Bprint(&bso, " %.8lux: %.8lux %.8lux %.8lux %.8lux %.8lux %.8lux%P\n",
                v, o1, o2, o3, o4, o5, o6, p);
        /*e: [[asmout()]] when 6 generated instructions, debug */
        lputl(o1);
        lputl(o2);
        lputl(o3);
        lputl(o4);
        lputl(o5);
        lputl(o6);
        break;
    default:
        /*s: [[asmout()]] when other size, debug */
        if(debug['a'])
            Bprint(&bso, " %.8lux:\t\t%P\n", v, p);
        /*e: [[asmout()]] when other size, debug */
        break;
    /*e: [[asmout()]] switch on size cases */
    }
}
/*e: function [[asmout]](arm) */

/*e: linkers/5l/codegen.c */
