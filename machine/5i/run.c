/*s: machine/5i/run.c */
/*s: basic includes */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

#include "arm.h"
/*e: basic includes */

#define XCAST(a) (uvlong)(ulong)a

void	undef(ulong);

void	Idp0(ulong);
void	Idp1(ulong);
void	Idp2(ulong);
void	Idp3(ulong);

void	Imul(ulong);
void	Imula(ulong);
void	Imull(ulong);

void	Iswap(ulong);
void	Imem1(ulong);
void	Imem2(ulong);
void	Ilsm(ulong inst);

void	Ib(ulong);
void	Ibl(ulong);

void	Ssyscall(ulong);

//static	int	dummy;

/*s: global shtype */
static	char*	shtype[4] =
{
    "<<",
    ">>",
    "->",
    "@>",
};
/*e: global shtype */
/*s: global cond */
static	char*	cond[16] =
{
    ".EQ",	".NE",	".HS",	".LO",
    ".MI",	".PL",	".VS",	".VC",
    ".HI",	".LS",	".GE",	".LT",
    ".GT",	".LE",	"",	".NO",
};
/*e: global cond */

/*s: global itab */
Inst itab[] =
{
  /*s: [[itab]] elements */
  [85] =  { undef,		"undef" },
  [86] =  { undef,		"undef" },
  [87] =  { undef,		"undef" },
  /*x: [[itab]] elements */
  [CUNDEF] =  { undef,		"undef"  },
  /*x: [[itab]] elements */
  [CMUL+0] =  { Imul,	"MUL",	Iarith },
  [CMUL+1] =  { Imula,	"MULA",	Iarith },	
  /*x: [[itab]] elements */
  [CMULTMP+0] =  { Imull,		"MULLU",	Iarith },
  [CMULTMP+1] =  { Imull,		"MULALU",	Iarith },
  [CMULTMP+2] =  { Imull,		"MULL",		Iarith  },
  [CMULTMP+3] =  { Imull,		"MULAL",	Iarith  },
  /*x: [[itab]] elements */
  //r,r,r
  [CARITH0+ 0] =  { Idp0,		"AND",	Iarith },	
  [CARITH0+ 1] =  { Idp0,		"EOR",	Iarith },	
  [CARITH0+ 2] =  { Idp0,		"SUB",	Iarith },	
  [CARITH0+ 3] =  { Idp0,		"RSB",	Iarith },	
  [CARITH0+ 4] =  { Idp0,		"ADD",	Iarith },	
  [CARITH0+ 5] =  { Idp0,		"ADC",	Iarith },	
  [CARITH0+ 6] =  { Idp0,		"SBC",	Iarith },	
  [CARITH0+ 7] =  { Idp0,		"RSC",	Iarith },	
  [CARITH0+ 8] =  { Idp0,		"TST",	Iarith },	
  [CARITH0+ 9] =  { Idp0,		"TEQ",	Iarith },	
  [CARITH0+10] =  { Idp0,		"CMP",	Iarith },	
  [CARITH0+11] =  { Idp0,		"CMN",	Iarith },	
  [CARITH0+12] =  { Idp0,		"ORR",	Iarith },	
  [CARITH0+13] =  { Idp0,		"MOV",	Iarith },	
  [CARITH0+14] =  { Idp0,		"BIC",	Iarith },	
  [CARITH0+15] =  { Idp0,		"MVN",	Iarith },	
  /*x: [[itab]] elements */
  [CARITH1+ 0] =  { Idp1,		"AND",	Iarith },	
  [CARITH1+ 1] =  { Idp1,		"EOR",	Iarith },	
  [CARITH1+ 2] =  { Idp1,		"SUB",	Iarith },	
  [CARITH1+ 3] =  { Idp1,		"RSB",	Iarith },	
  [CARITH1+ 4] =  { Idp1,		"ADD",	Iarith },	
  [CARITH1+ 5] =  { Idp1,		"ADC",	Iarith },	
  [CARITH1+ 6] =  { Idp1,		"SBC",	Iarith },	
  [CARITH1+ 7] =  { Idp1,		"RSC",	Iarith },	
  [CARITH1+ 8] =  { Idp1,		"TST",	Iarith },	
  [CARITH1+ 9] =  { Idp1,		"TEQ",	Iarith },	
  [CARITH1+10] =  { Idp1,		"CMP",	Iarith },	
  [CARITH1+11] =  { Idp1,		"CMN",	Iarith },	
  [CARITH1+12] =  { Idp1,		"ORR",	Iarith },	
  [CARITH1+13] =  { Idp1,		"MOV",	Iarith },	
  [CARITH1+14] =  { Idp1,		"BIC",	Iarith },	
  [CARITH1+15] =  { Idp1,		"MVN",	Iarith },	
  /*x: [[itab]] elements */
  [CARITH2+ 0] =  { Idp2,		"AND",	Iarith },	
  [CARITH2+ 1] =  { Idp2,		"EOR",	Iarith },	
  [CARITH2+ 2] =  { Idp2,		"SUB",	Iarith },	
  [CARITH2+ 3] =  { Idp2,		"RSB",	Iarith },	
  [CARITH2+ 4] =  { Idp2,		"ADD",	Iarith },	
  [CARITH2+ 5] =  { Idp2,		"ADC",	Iarith },	
  [CARITH2+ 6] =  { Idp2,		"SBC",	Iarith },	
  [CARITH2+ 7] =  { Idp2,		"RSC",	Iarith },	
  [CARITH2+ 8] =  { Idp2,		"TST",	Iarith },	
  [CARITH2+ 9] =  { Idp2,		"TEQ",	Iarith },	
  [CARITH2+10] =  { Idp2,		"CMP",	Iarith },	
  [CARITH2+11] =  { Idp2,		"CMN",	Iarith },	
  [CARITH2+12] =  { Idp2,		"ORR",	Iarith },	
  [CARITH2+13] =  { Idp2,		"MOV",	Iarith },	
  [CARITH2+14] =  { Idp2,		"BIC",	Iarith },	
  [CARITH2+15] =  { Idp2,		"MVN",	Iarith },	
  /*x: [[itab]] elements */
  //i,r,r
  [CARITH3+ 0] =  { Idp3,		"AND",	Iarith },	
  [CARITH3+ 1] =  { Idp3,		"EOR",	Iarith },	
  [CARITH3+ 2] =  { Idp3,		"SUB",	Iarith },	
  [CARITH3+ 3] =  { Idp3,		"RSB",	Iarith },	
  [CARITH3+ 4] =  { Idp3,		"ADD",	Iarith },	
  [CARITH3+ 5] =  { Idp3,		"ADC",	Iarith },	
  [CARITH3+ 6] =  { Idp3,		"SBC",	Iarith },	
  [CARITH3+ 7] =  { Idp3,		"RSC",	Iarith },	
  [CARITH3+ 8] =  { Idp3,		"TST",	Iarith },	
  [CARITH3+ 9] =  { Idp3,		"TEQ",	Iarith },	
  [CARITH3+10] =  { Idp3,		"CMP",	Iarith },	
  [CARITH3+11] =  { Idp3,		"CMN",	Iarith },	
  [CARITH3+12] =  { Idp3,		"ORR",	Iarith },	
  [CARITH3+13] =  { Idp3,		"MOV",	Iarith },	
  [CARITH3+14] =  { Idp3,		"BIC",	Iarith },	
  [CARITH3+15] =  { Idp3,		"MVN",	Iarith },	
  /*x: [[itab]] elements */
  [CSWAP+0] =  { Iswap,		"SWPW",	Imem },
  [CSWAP+1] =  { Iswap,		"SWPBU",Imem },
  /*x: [[itab]] elements */
  // load/store h/sb
  [CMEM+0] =  { Imem2,		"MOV",	Imem },
  [CMEM+1] =  { Imem2,		"MOV",	Imem },
  [CMEM+2] =  { Imem2,		"MOV",	Imem },
  [CMEM+3] =  { Imem2,		"MOV",	Imem },
  /*x: [[itab]] elements */
  // load/store w/ub i,r
  [CMEM+4+0] =  { Imem1,		"MOVW",	Imem },
  [CMEM+4+1] =  { Imem1,		"MOVB",	Imem },
  [CMEM+4+2] =  { Imem1,		"MOVW",	Imem },
  [CMEM+4+3] =  { Imem1,		"MOVB",	Imem },
  /*x: [[itab]] elements */
  // load/store r,r
  [CMEM+8+0] =  { Imem1,		"MOVW",	Imem },
  [CMEM+8+1] =  { Imem1,		"MOVB",	Imem },
  [CMEM+8+2] =  { Imem1,		"MOVW",	Imem },
  [CMEM+8+3] =  { Imem1,		"MOVB",	Imem },
  /*x: [[itab]] elements */
  // block move r,r
  [CBLOC+0] =  { Ilsm,		"LDM",	Imem },
  [CBLOC+1] =  { Ilsm,		"STM",	Imem },
  /*x: [[itab]] elements */
  // branch
  [CBRANCH+0] =  { Ib,	"B",	Ibranch },
  [CBRANCH+1] =  { Ibl,	"BL",	Ibranch },
  /*x: [[itab]] elements */
  // co processor
  [CSYSCALL] =  { Ssyscall,		"SWI",	Isyscall },
  /*e: [[itab]] elements */
  { 0 }
};
/*e: global itab */


/*s: function runcmp */
bool
runcmp(void)
{
    switch(reg.cond) {
    case 0x0:	/* eq */	return (reg.cc1 == reg.cc2);
    case 0x1:	/* ne */	return (reg.cc1 != reg.cc2);
    case 0x2:	/* hs */	return ((ulong)reg.cc1 >= (ulong)reg.cc2);
    case 0x3:	/* lo */	return ((ulong)reg.cc1 < (ulong)reg.cc2);
    case 0x4:	/* mi */	return (reg.cc1 - reg.cc2 < 0);
    case 0x5:	/* pl */	return (reg.cc1 - reg.cc2 >= 0);
    case 0x8:	/* hi */	return ((ulong)reg.cc1 > (ulong)reg.cc2);
    case 0x9:	/* ls */	return ((ulong)reg.cc1 <= (ulong)reg.cc2);
    case 0xa:	/* ge */	return (reg.cc1 >= reg.cc2);
    case 0xb:	/* lt */	return (reg.cc1 < reg.cc2);
    case 0xc:	/* gt */	return (reg.cc1 > reg.cc2);
    case 0xd:	/* le */	return (reg.cc1 <= reg.cc2);

    case 0xe:	/* al */	return true;
    case 0xf:	/* nv */	return false;
    default:
        Bprint(bioout, "unimplemented condition prefix %x (%ld %ld)\n",
            reg.cond, reg.cc1, reg.cc2);
        undef(reg.ir);
        return false;
    }
}
/*e: function runcmp */

/*s: function runteq */
bool
runteq(void)
{
    long res = reg.cc1 ^ reg.cc2;

    switch(reg.cond) {
    case 0x0:	/* eq */	return res == 0;
    case 0x1:	/* ne */	return res != 0;
    case 0x4:	/* mi */	return (res & SIGNBIT) != 0;
    case 0x5:	/* pl */	return (res & SIGNBIT) == 0;
    case 0xe:	/* al */	return true;
    case 0xf:	/* nv */	return false;
    default:
        Bprint(bioout, "unimplemented condition prefix %x (%ld %ld)\n",
            reg.cond, reg.cc1, reg.cc2);
        undef(reg.ir);
        return false;
    }
}
/*e: function runteq */

/*s: function runtst */
bool
runtst(void)
{
    long res = reg.cc1 & reg.cc2;

    switch(reg.cond) {
    case 0x0:	/* eq */	return res == 0;
    case 0x1:	/* ne */	return res != 0;
    case 0x4:	/* mi */	return (res & SIGNBIT) != 0;
    case 0x5:	/* pl */	return (res & SIGNBIT) == 0;
    case 0xe:	/* al */	return true;
    case 0xf:	/* nv */	return false;
    default:
        Bprint(bioout, "unimplemented condition prefix %x (%ld %ld)\n",
            reg.cond, reg.cc1, reg.cc2);
        undef(reg.ir);
        return false;
    }
}
/*e: function runtst */

/*s: function run */
void
run(void)
{
    bool execute;

    do {
        if(trace) Bflush(bioout);

        reg.ar = reg.r[REGPC];
        reg.ir = ifetch(reg.ar);

        reg.class = arm_class(reg.ir);
        reg.ip = &itab[reg.class];

        /*s: [[run()]] set reg.cond */
        reg.cond = (reg.ir>>28) & 0xf;
        /*e: [[run()]] set reg.cond */
        /*s: [[run()]] switch reg.compare_op to set execute */
        switch(reg.compare_op) {
        case CCcmp:
            execute = runcmp(); // use reg.cond
            break;
        case CCteq:
            execute = runteq();
            break;
        case CCtst:
            execute = runtst();
            break;
        default:
            Bprint(bioout, "unimplemented compare operation %x\n",
                reg.compare_op);
            return;
        }
        /*e: [[run()]] switch reg.compare_op to set execute */

        if(execute) {
            /*s: [[run()]] profile current instruction class */
            // profiling
            reg.ip->count++;
            /*e: [[run()]] profile current instruction class */
            // !!the dispatch!!
            (*reg.ip->func)(reg.ir);

        }
        else
          if(trace) itrace("%s%s IGNORED",reg.ip->name, cond[reg.cond]);

        reg.r[REGPC] += 4; // simple archi with fixed-length instruction :)

        /*s: [[run()]] check for breakpoints */
        if(bplist)
            brkchk(reg.r[REGPC], Instruction);
        /*e: [[run()]] check for breakpoints */
    } while(--count);
}
/*e: function run */

/*s: function undef */
void
undef(instruction inst)
{
    Bprint(bioout, "undefined instruction trap pc #%lux inst %.8lux class %d\n",
        reg.r[REGPC], inst, reg.class);
    longjmp(errjmp, 0);
}
/*e: function undef */

/*s: function arm_class */
int
arm_class(instruction w)
{
    int op;
   
    op = (w >> 25) & 0x7;
    switch(op) {
    /*s: [[arm_class()]] op cases */
    case 0:	/* data processing r,r,r */
        op = ((w >> 4) & 0xf);
        /*s: [[arm_class()]] class 0, if op == 0x9 */
        /* mul, swp, mull */
        if(op == 0x9) {
            op = CMUL;
            /*s: [[arm_class()]] class 0, when op == 0x9, if 24 bit set */
            if(w & (1<<24)) {
                op = CSWAP;
                if(w & (1<<22))
                     op++;	/* swpb */
                break;
            }
            /*e: [[arm_class()]] class 0, when op == 0x9, if 24 bit set */
            if(w & (1<<23)) {	/* mullu */
                op = CMULTMP;
                if(w & (1<<22))	/* mull */
                    op += 2;
            }
            if(w & (1<<21))
                op++;		/* mla */
            break;
        }
        /*e: [[arm_class()]] class 0, if op == 0x9 */
        /*s: [[arm_class()]] class 0, if op & 0x9 == 0x9 */
        if((op & 0x9) == 0x9) {		/* ld/st byte/half s/u */
             op = CMEM + ((w >> 22) & 0x1) + ((w >> 19) & 0x2);
             break;
        }
        /*e: [[arm_class()]] class 0, if op & 0x9 == 0x9 */
   
        op = (w >> 21) & 0xf;

        if(w & (1<<4))
          op += CARITH2;
        else
         if((w & (31<<7)) || (w & (1<<5)))
          op += CARITH1;
        // else op+=CARITH0 (= 0)
        break;
    /*x: [[arm_class()]] op cases */
    case 1:	/* data processing i,r,r */
     op = CARITH3 + ((w >> 21) & 0xf);
     break;
    /*x: [[arm_class()]] op cases */
    case 2:	/* load/store byte/word i(r) */
     op = CMEM+4 + ((w >> 22) & 0x1) + ((w >> 19) & 0x2);
     break;
    /*x: [[arm_class()]] op cases */
    case 3:	/* load/store byte/word (r)(r) */
     op = CMEM+8 + ((w >> 22) & 0x1) + ((w >> 19) & 0x2);
     break;
    /*x: [[arm_class()]] op cases */
    case 4:	/* block data transfer (r)(r) */
     op = CBLOC + ((w >> 20) & 0x1);
     break;
    /*x: [[arm_class()]] op cases */
    case 5:	/* branch / branch link */
     op = CBRANCH + ((w >> 24) & 0x1);
     break;
    /*x: [[arm_class()]] op cases */
    case 7:	/* coprocessor crap */ // and syscall
     if((w >> 25) & 0x1)
       op = CSYSCALL;
     else
       op = CUNDEF; // coprocessor stuff not handled
     break;
    /*e: [[arm_class()]] op cases */
    default:	  
        op = CUNDEF;
        break;
    }
    return op;
}
/*e: function arm_class */

/*s: function shift */
long
shift(long v, int st, int sc, bool isreg)
{
    if(sc == 0) {
        switch(st) {
        case 0:	/* logical left */
            reg.cout = reg.cbit;
            break;
        case 1:	/* logical right */
            reg.cout = (v >> 31) & 1;
            break;
        case 2:	/* arith right */
            reg.cout = reg.cbit;
            break;
        case 3:	/* rotate right */
            if(isreg) {
                reg.cout = reg.cbit;
            }
            else {
                reg.cout = v & 1;
                v = ((ulong)v >> 1) | (reg.cbit << 31);
            }
        }
    }
    else {
        switch(st) {
        case 0:	/* logical left */
            reg.cout = (v >> (32 - sc)) & 1;
            v = v << sc;
            break;
        case 1:	/* logical right */
            reg.cout = (v >> (sc - 1)) & 1;
            v = (ulong)v >> sc;
            break;
        case 2:	/* arith right */
            if(sc >= 32) {
                reg.cout = (v >> 31) & 1;
                if(reg.cout)
                    v = 0xFFFFFFFF;
                else
                    v = 0;
            }
            else {
                reg.cout = (v >> (sc - 1)) & 1;
                v = (long)v >> sc;
            }
            break;
        case 3:	/* rotate right */
            reg.cout = (v >> (sc - 1)) & 1;
            v = (v << (32-sc)) | ((ulong)v >> sc);
            break;
        }
    }
    return v;
}
/*e: function shift */

/*s: function dpex */
void
dpex(instruction inst, long o1, long o2, int rd)
{
    bool cbit;

    cbit = false;

    switch((inst>>21) & 0xf) {
    case  0:	/* and */
        reg.r[rd] = o1 & o2;
        cbit = true;
        break;
    case  1:	/* eor */
        reg.r[rd] = o1 ^ o2;
        cbit = true;
        break;

    case  2:	/* sub */
        reg.r[rd] = o1 - o2;
        // Fallthrough
    case 10:	/* cmp */
        if(inst & Sbit) {
            reg.cc1 = o1;
            reg.cc2 = o2;
            reg.compare_op = CCcmp;
        }
        return;

    case  3:	/* rsb */
        reg.r[rd] = o2 - o1;
        if(inst & Sbit) {
            reg.cc1 = o2;
            reg.cc2 = o1;
            reg.compare_op = CCcmp;
        }
        return;
    case  4:	/* add */
        /*s: [[dpex()]] if calltree, when add operation */
        if(calltree && rd == REGPC && o2 == 0) {
            Symbol s;

            findsym(o1 + o2, CTEXT, &s);
            Bprint(bioout, "%8lux return to %lux %s r0=%lux\n",
                        reg.r[REGPC], o1 + o2, s.name, reg.r[REGRET]);
        }
        /*e: [[dpex()]] if calltree, when add operation */
        reg.r[rd] = o1 + o2;
        if(inst & Sbit) {
            if((XCAST(o1) + XCAST(o2)) & (1LL << 32))
                reg.cbit = true;
            else
                reg.cbit = false;
            reg.cc1 = o2;
            reg.cc2 = -o1;
            reg.compare_op = CCcmp;
        }
        return;

    case  5:	/* adc */
    case  6:	/* sbc */
    case  7:	/* rsc */
        undef(inst);

    case  8:	/* tst */
        if(inst & Sbit) {
            reg.cc1 = o1;
            reg.cc2 = o2;
            reg.compare_op = CCtst;
        }
        return;
    case  9:	/* teq */
        if(inst & Sbit) { // not always true?
            reg.cc1 = o1;
            reg.cc2 = o2;
            reg.compare_op = CCteq;
        }
        return;
    case 11:	/* cmn */
        if(inst & Sbit) { // not always true?
            reg.cc1 = o1;
            reg.cc2 = -o2;
            reg.compare_op = CCcmp;
        }
        return;

    case 12:	/* orr */
        reg.r[rd] = o1 | o2;
        cbit = true;
        break;
    case 13:	/* mov */
        reg.r[rd] = o2;
        cbit = true;
        break;
    case 14:	/* bic */
        reg.r[rd] = o1 & ~o2;
        cbit = true;
        break;
    case 15:	/* mvn */
        reg.r[rd] = ~o2;
        cbit = true;
        break;
    }

    if(inst & Sbit) {
        if(cbit)
            reg.cbit = reg.cout;
        reg.cc1 = reg.r[rd];
        reg.cc2 = 0;
        reg.compare_op = CCcmp;
    }
}
/*e: function dpex */

/*s: function Idp0 */
/*
 * data processing instruction R,R,R
 */
void
Idp0(instruction inst)
{
    int rn, rd, rm;
    long o1, o2;

    rn = (inst>>16) & 0xf;
    rd = (inst>>12) & 0xf;
    rm = inst & 0xf;

    o1 = reg.r[rn];
    if(rn == REGPC)
        o1 += 8;

    o2 = reg.r[rm];
    if(rm == REGPC)
        o2 += 8;

    dpex(inst, o1, o2, rd);

    /*s: [[Idp0()]] trace */
    if(trace)
        itrace("%s%s\tR%d,R%d,R%d =#%x",
            reg.ip->name, cond[reg.cond],
            rm, rn, rd,
            reg.r[rd]);
    /*e: [[Idp0()]] trace */
    /*s: [[Idpx()]] compensate REGPC */
    if(rd == REGPC)
        reg.r[rd] -= 4;
    /*e: [[Idpx()]] compensate REGPC */
}
/*e: function Idp0 */

/*s: function Idp1 */
/*
 * data processing instruction (R<>#),R,R
 */
void
Idp1(instruction inst)
{
    int rn, rd, rm, st, sc;
    long o1, o2;

    rn = (inst>>16) & 0xf;
    rd = (inst>>12) & 0xf;
    rm = inst & 0xf;
    st = (inst>>5) & 0x3;
    sc = (inst>>7) & 0x1f;

    o1 = reg.r[rn];
    if(rn == REGPC)
        o1 += 8;

    o2 = reg.r[rm];
    if(rm == REGPC)
        o2 += 8;

    o2 = shift(o2, st, sc, false);

    dpex(inst, o1, o2, rd);

    /*s: [[Idp1()]] trace */
    if(trace)
        itrace("%s%s\tR%d%s%d,R%d,R%d =#%x",
            reg.ip->name, cond[reg.cond], rm, shtype[st], sc, rn, rd,
            reg.r[rd]);
    /*e: [[Idp1()]] trace */
    /*s: [[Idpx()]] compensate REGPC */
    if(rd == REGPC)
        reg.r[rd] -= 4;
    /*e: [[Idpx()]] compensate REGPC */
}
/*e: function Idp1 */

/*s: function Idp2 */
/*
 * data processing instruction (R<>R),R,R
 */
void
Idp2(instruction inst)
{
    int rn, rd, rm, rs, st;
    long o1, o2, o3;

    rn = (inst>>16) & 0xf;
    rd = (inst>>12) & 0xf;
    rm = inst & 0xf;
    st = (inst>>5) & 0x3;
    rs = (inst>>8) & 0xf;

    o1 = reg.r[rn];
    if(rn == REGPC)
        o1 += 8;

    o2 = reg.r[rm];
    if(rm == REGPC)
        o2 += 8;

    o3 = reg.r[rs];
    if(rs == REGPC)
        o3 += 8;

    o2 = shift(o2, st, o3, true);

    dpex(inst, o1, o2, rd);

    /*s: [[Idp2()]] trace */
    if(trace)
        itrace("%s%s\tR%d%sR%d=%d,R%d,R%d =#%x",
            reg.ip->name, cond[reg.cond], rm, shtype[st], rs, o3, rn, rd,
            reg.r[rd]);
    /*e: [[Idp2()]] trace */
    /*s: [[Idpx()]] compensate REGPC */
    if(rd == REGPC)
        reg.r[rd] -= 4;
    /*e: [[Idpx()]] compensate REGPC */
}
/*e: function Idp2 */

/*s: function Idp3 */
/*
 * data processing instruction #<>#,R,R
 */
void
Idp3(instruction inst)
{
    int rn, rd, sc;
    long o1, o2;

    rn = (inst>>16) & 0xf;
    rd = (inst>>12) & 0xf;
    o1 = reg.r[rn];
    if(rn == REGPC)
        o1 += 8;

    o2 = inst & 0xff;
    sc = (inst>>7) & 0x1e;
    o2 = (o2 >> sc) | (o2 << (32 - sc));

    dpex(inst, o1, o2, rd);

    /*s: [[Idp3()]] trace */
    if(trace)
        itrace("%s%s\t#%x,R%d,R%d =#%x",
            reg.ip->name, cond[reg.cond], o2, rn, rd,
            reg.r[rd]);
    /*e: [[Idp3()]] trace */
    /*s: [[Idpx()]] compensate REGPC */
    if(rd == REGPC)
        reg.r[rd] -= 4;
    /*e: [[Idpx()]] compensate REGPC */
}
/*e: function Idp3 */

/*s: function Imul */
void
Imul(instruction inst)
{
    int rs, rd, rm;

    rd = (inst>>16) & 0xf;
    rs = (inst>>8) & 0xf;
    rm = inst & 0xf;

    if(rd == REGPC || rs == REGPC || rm == REGPC || rd == rm)
        undef(inst);

    reg.r[rd] = reg.r[rm]*reg.r[rs];

    /*s: [[Imul()]] trace */
    if(trace)
        itrace("%s%s\tR%d,R%d,R%d =#%x",
            reg.ip->name, cond[reg.cond], rs, rm, rd,
            reg.r[rd]);
    /*e: [[Imul()]] trace */
}
/*e: function Imul */

/*s: function Imull */
void
Imull(instruction inst)
{
    vlong v;
    int rs, rd, rm, rn;

    rd = (inst>>16) & 0xf;
    rn = (inst>>12) & 0xf;
    rs = (inst>>8) & 0xf;
    rm = inst & 0xf;

    if(rd == REGPC || rn == REGPC || rs == REGPC || rm == REGPC
      || rd == rm || rn == rm || rd == rn
      )
        undef(inst);

    if(inst & (1<<22)){
        v = (vlong)reg.r[rm] * (vlong)reg.r[rs];
        if(inst & (1 << 21))
            v += reg.r[rn];
    }else{
        v = XCAST(reg.r[rm]) * XCAST(reg.r[rs]);
        if(inst & (1 << 21))
            v += (ulong)reg.r[rn];
    }
    reg.r[rd] = v >> 32;
    reg.r[rn] = v;

    /*s: [[Imull()]] trace */
    if(trace)
        itrace("%s%s\tR%d,R%d,(R%d,R%d) =#%llx",
            reg.ip->name, cond[reg.cond], rs, rm, rn, rd,
            v);
    /*e: [[Imull()]] trace */
}
/*e: function Imull */

/*s: function Imula */
void
Imula(instruction inst)
{
    int rs, rd, rm, rn;

    rd = (inst>>16) & 0xf;
    rn = (inst>>12) & 0xf;
    rs = (inst>>8) & 0xf;
    rm = inst & 0xf;

    if(rd == REGPC || rn == REGPC || rs == REGPC || rm == REGPC || rd == rm)
        undef(inst);

    reg.r[rd] = reg.r[rm]*reg.r[rs] + reg.r[rn];

    /*s: [[Imula()]] trace */
    if(trace)
        itrace("%s%s\tR%d,R%d,R%d,R%d =#%x",
            reg.ip->name, cond[reg.cond], rs, rm, rn, rd,
            reg.r[rd]);
    /*e: [[Imula()]] trace */
}
/*e: function Imula */

/*s: function Iswap */
void
Iswap(instruction inst)
{
    int rn, rd, rm;
    ulong address, value, bbit;

    bbit = inst & (1<<22);
    rn = (inst>>16) & 0xf;
    rd = (inst>>12) & 0xf;
    rm = (inst>>0) & 0xf;

    address = reg.r[rn];
    if(bbit) {
        value = getmem_b(address);
        putmem_b(address, reg.r[rm]);
    } else {
        value = getmem_w(address);
        putmem_w(address, reg.r[rm]);
    }
    reg.r[rd] = value;

    /*s: [[Iswap()]] trace */
    if(trace) {
        char *bw, *dotc;

        bw = "";
        if(bbit)
            bw = "B";
        dotc = cond[reg.cond];

        itrace("SWP%s%s\t#%x(R%d),R%d #%lux=#%x",
            bw, dotc,
            rn, rd,
            address, value);
    }
    /*e: [[Iswap()]] trace */
}
/*e: function Iswap */

/*s: function Imem1 */
/*
 * load/store word/byte
 */
void
Imem1(instruction inst)
{
    int rn, rd, off, rm, sc, st;
    ulong address, value, pbit, ubit, bbit, wbit, lbit, bit25;

    bit25 = inst & (1<<25);
    pbit = inst & (1<<24);
    ubit = inst & (1<<23);
    bbit = inst & (1<<22);
    wbit = inst & (1<<21);
    lbit = inst & (1<<20);
    rn = (inst>>16) & 0xf;
    rd = (inst>>12) & 0xf;

    SET(st);
    SET(sc);
    SET(rm);
    if(bit25) {
        rm = inst & 0xf;
        st = (inst>>5) & 0x3;
        sc = (inst>>7) & 0x1f;
        off = reg.r[rm];
        if(rm == REGPC)
            off += 8;
        off = shift(off, st, sc, false);
    } else {
        off = inst & 0xfff;
    }
    if(!ubit)
        off = -off;
    if(rn == REGPC)
        off += 8;

    address = reg.r[rn];
    if(pbit)
        address += off;

    if(lbit) {
        if(bbit)
            value = getmem_b(address);
        else
            value = getmem_w(address);
        if(rd == REGPC)
            value -= 4;
        reg.r[rd] = value;
    } else {
        value = reg.r[rd];
        if(rd == REGPC)
            value -= 4;
        if(bbit)
            putmem_b(address, value);
        else
            putmem_w(address, value);
    }
    if(!(pbit && !wbit))
        reg.r[rn] += off;

    /*s: [[Imem1()]] trace */
    if(trace) {
        char *bw, *dotp, *dotc;

        bw = "W";
        if(bbit)
            bw = "BU";
        dotp = "";
        if(!pbit)
            dotp = ".P";
        dotc = cond[reg.cond];

        if(lbit) {
            if(!bit25)
                itrace("MOV%s%s%s\t#%x(R%d),R%d #%lux=#%x",
                    bw, dotp, dotc,
                    off, rn, rd,
                    address, value);
            else
                itrace("MOV%s%s%s\t(R%d%s%d)(R%d),R%d  #%lux=#%x",
                    bw, dotp, dotc,
                    rm, shtype[st], sc, rn, rd,
                    address, value);
        } else {
            if(!bit25)
                itrace("MOV%s%s%s\tR%d,#%x(R%d) #%lux=#%x",
                    bw, dotp, dotc,
                    rd, off, rn,
                    address, value);
            else
                itrace("MOV%s%s%s\tR%d,(R%d%s%d)(R%d) #%lux=#%x",
                    bw, dotp, dotc,
                    rd, rm, shtype[st], sc, rn,
                    address, value);
        }
    }
    /*e: [[Imem1()]] trace */
}
/*e: function Imem1 */

/*s: function Imem2 */
/*
 * load/store unsigned byte/half word
 */
void
Imem2(instruction inst)
{
    int rn, rd, off, rm;
    ulong address, value, pbit, ubit, hbit, sbit, wbit, lbit, bit22;

    pbit = inst & (1<<24);
    ubit = inst & (1<<23);
    bit22 = inst & (1<<22);
    wbit = inst & (1<<21);
    lbit = inst & (1<<20);
    sbit = inst & (1<<6);
    hbit = inst & (1<<5);
    rn = (inst>>16) & 0xf;
    rd = (inst>>12) & 0xf;

    SET(rm);
    if(bit22) {
        off = ((inst>>4) & 0xf0) | (inst & 0xf);
    } else {
        rm = inst & 0xf;
        off = reg.r[rm];
        if(rm == REGPC)
            off += 8;
    }
    if(!ubit)
        off = -off;
    if(rn == REGPC)
        off += 8;

    address = reg.r[rn];
    if(pbit)
        address += off;

    if(lbit) {
        if(hbit) {
            value = getmem_h(address);
            if(sbit && (value & 0x8000))
                value |= 0xffff0000;
        } else {
            value = getmem_b(address);
            if(value & 0x80)
                value |= 0xffffff00;
        }
        if(rd == REGPC)
            value -= 4;
        reg.r[rd] = value;
    } else {
        value = reg.r[rd];
        if(rd == REGPC)
            value -= 4;
        if(hbit) {
            putmem_h(address, value);
        } else {
            putmem_b(address, value);
        }
    }
    if(!(pbit && !wbit))
        reg.r[rn] += off;

    /*s: [[Imem2()]] trace */
    if(trace) {
        char *hb, *dotp, *dotc;

        hb = "B";
        if(hbit)
            hb = "H";
        dotp = "";
        if(!pbit)
            dotp = ".P";
        dotc = cond[reg.cond];

        if(lbit) {
            if(bit22)
                itrace("MOV%s%s%s\t#%x(R%d),R%d #%lux=#%x",
                    hb, dotp, dotc,
                    off, rn, rd,
                    address, value);
            else
                itrace("MOV%s%s%s\t(R%d)(R%d),R%d  #%lux=#%x",
                    hb, dotp, dotc,
                    rm, rn, rd,
                    address, value);
        } else {
            if(bit22)
                itrace("MOV%s%s%s\tR%d,#%x(R%d) #%lux=#%x",
                    hb, dotp, dotc,
                    rd, off, rn,
                    address, value);
            else
                itrace("MOV%s%s%s\tR%d,(R%d)(R%d) #%lux=#%x",
                    hb, dotp, dotc,
                    rd, rm, rn,
                    address, value);
        }
    }
    /*e: [[Imem2()]] trace */
}
/*e: function Imem2 */

/*s: function Ilsm */
void
Ilsm(instruction inst)
{
    char pbit, ubit, sbit, wbit, lbit;
    int i, rn, reglist;
    ulong address, predelta, postdelta;

    pbit = (inst>>24) & 0x1;
    ubit = (inst>>23) & 0x1;
    sbit = (inst>>22) & 0x1;
    wbit = (inst>>21) & 0x1;
    lbit = (inst>>20) & 0x1;
    rn =   (inst>>16) & 0xf;
    reglist = inst & 0xffff;

    if(reglist & 0x8000)
        undef(reg.ir);
    if(sbit)
        undef(reg.ir);

    address = reg.r[rn];

    if(pbit) {
        predelta = 4;
        postdelta = 0;
    } else {
        predelta = 0;
        postdelta = 4;
    }
    if(ubit) {
        for (i = 0; i < 16; ++i) {
            if(!(reglist & (1 << i)))
                continue;
            address += predelta;
            if(lbit)
                reg.r[i] = getmem_w(address);
            else
                putmem_w(address, reg.r[i]);
            address += postdelta;
        }
    } else {
        for (i = 15; 0 <= i; --i) {
            if(!(reglist & (1 << i)))
                continue;
            address -= predelta;
            if(lbit)
                reg.r[i] = getmem_w(address);
            else
                putmem_w(address, reg.r[i]);
            address -= postdelta;
        }
    }
    if(wbit) {
        reg.r[rn] = address;
    }

    /*s: [[Ilsm()]] trace */
    if(trace) {
        itrace("%s.%c%c\tR%d=%lux%s, <%lux>",
            (lbit ? "LDM" : "STM"), (ubit ? 'I' : 'D'), (pbit ? 'B' : 'A'),
            rn, reg.r[rn], (wbit ? "!" : ""), reglist);
    }
    /*e: [[Ilsm()]] trace */
}
/*e: function Ilsm */

/*s: function Ib */
void
Ib(instruction inst)
{
    long v;

    v = inst & 0xffffff; // 24 bits
    v = reg.r[REGPC] + 8 + ((v << 8) >> 6);
    /*s: [[Ib()]] trace */
    if(trace)
        itrace("B%s\t#%lux", cond[reg.cond], v);
    /*e: [[Ib()]] trace */
    reg.r[REGPC] = v - 4;
}
/*e: function Ib */

/*s: function Ibl */
void
Ibl(instruction inst)
{
    long v;
    Symbol s;

    v = inst & 0xffffff;
    v = reg.r[REGPC] + 8 + ((v << 8) >> 6);
    /*s: [[Ibl()]] trace */
    if(trace)
        itrace("BL%s\t#%lux", cond[reg.cond], v);
    /*e: [[Ibl()]] trace */

    /*s: [[Ibl()]] if calltree */
    if(calltree) {
        findsym(v, CTEXT, &s);
        Bprint(bioout, "%8lux %s(", reg.r[REGPC], s.name);
        printparams(&s, reg.r[REGSP]);
        Bprint(bioout, "from ");
        printsource(reg.r[REGPC]);
        Bputc(bioout, '\n');
    }
    /*e: [[Ibl()]] if calltree */

    reg.r[REGLINK] = reg.r[REGPC] + 4;
    reg.r[REGPC] = v - 4;
}
/*e: function Ibl */
/*e: machine/5i/run.c */
