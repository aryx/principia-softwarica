/*s: machine/5i/run.c */
/*s: basic includes */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

#include "arm.h"
/*e: basic includes */

/*s: macro [[XCAST]] */
#define XCAST(a) (uvlong)(ulong)a
/*e: macro [[XCAST]] */

// forward decl
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

int arm_class(instruction w);

//static	int	dummy;

/*s: global [[shtype]] */
static	char*	shtype[4] =
{
    "<<",
    ">>",
    "->",
    "@>",
};
/*e: global [[shtype]] */
/*s: global [[cond]] */
static	char*	cond[16] =
{
    ".EQ",	".NE",	".HS",	".LO",
    ".MI",	".PL",	".VS",	".VC",
    ".HI",	".LS",	".GE",	".LT",
    ".GT",	".LE",	"",	".NO",
};
/*e: global [[cond]] */

/*s: global [[itab]] */
//map<enum<opcode>, Inst>
Inst itab[] =
{
  /*s: [[itab]] elements */
  [OUNDEF] =  { &undef,		"UNDEF", Imisc},
  /*x: [[itab]] elements */
  [OMUL]    =  { Imul,	"MUL",	Iarith },
  [OMULA]   =  { Imula,	"MULA",	Iarith },	
  /*x: [[itab]] elements */
  //r,r,r
  [OAND] =  { Idp0,		"AND",	Iarith },	
  [OEOR] =  { Idp0,		"EOR",	Iarith },	
  [OSUB] =  { Idp0,		"SUB",	Iarith },	
  [ORSB] =  { Idp0,		"RSB",	Iarith },	
  [OADD] =  { Idp0,		"ADD",	Iarith },	
  [OADC] =  { Idp0,		"ADC",	Iarith },	
  [OSBC] =  { Idp0,		"SBC",	Iarith },	
  [ORSC] =  { Idp0,		"RSC",	Iarith },	
  [OTST] =  { Idp0,		"TST",	Iarith },	
  [OTEQ] =  { Idp0,		"TEQ",	Iarith },	
  [OCMP] =  { Idp0,		"CMP",	Iarith },	
  [OCMN] =  { Idp0,		"CMN",	Iarith },	
  [OORR] =  { Idp0,		"ORR",	Iarith },	
  [OMOV] =  { Idp0,		"MOV",	Iarith },	
  [OBIC] =  { Idp0,		"BIC",	Iarith },	
  [OMVN] =  { Idp0,		"MVN",	Iarith },	
  /*x: [[itab]] elements */
  // r<>#, r, r
  [OAND +CARITH1] =  { Idp1,		"AND",	Iarith },	
  [OEOR +CARITH1] =  { Idp1,		"EOR",	Iarith },	
  [OSUB +CARITH1] =  { Idp1,		"SUB",	Iarith },	
  [ORSB +CARITH1] =  { Idp1,		"RSB",	Iarith },	
  [OADD +CARITH1] =  { Idp1,		"ADD",	Iarith },	
  [OADC +CARITH1] =  { Idp1,		"ADC",	Iarith },	
  [OSBC +CARITH1] =  { Idp1,		"SBC",	Iarith },	
  [ORSC +CARITH1] =  { Idp1,		"RSC",	Iarith },	
  [OTST +CARITH1] =  { Idp1,		"TST",	Iarith },	
  [OTEQ +CARITH1] =  { Idp1,		"TEQ",	Iarith },	
  [OCMP +CARITH1] =  { Idp1,		"CMP",	Iarith },	
  [OCMN +CARITH1] =  { Idp1,		"CMN",	Iarith },	
  [OORR +CARITH1] =  { Idp1,		"ORR",	Iarith },	
  [OMOV +CARITH1] =  { Idp1,		"MOV",	Iarith },	
  [OBIC +CARITH1] =  { Idp1,		"BIC",	Iarith },	
  [OMVN +CARITH1] =  { Idp1,		"MVN",	Iarith },	
  /*x: [[itab]] elements */
  // r<>r, r, r
  [OAND +CARITH2] =  { Idp2,		"AND",	Iarith },	
  [OEOR +CARITH2] =  { Idp2,		"EOR",	Iarith },	
  [OSUB +CARITH2] =  { Idp2,		"SUB",	Iarith },	
  [ORSB +CARITH2] =  { Idp2,		"RSB",	Iarith },	
  [OADD +CARITH2] =  { Idp2,		"ADD",	Iarith },	
  [OADC +CARITH2] =  { Idp2,		"ADC",	Iarith },	
  [OSBC +CARITH2] =  { Idp2,		"SBC",	Iarith },	
  [ORSC +CARITH2] =  { Idp2,		"RSC",	Iarith },	
  [OTST +CARITH2] =  { Idp2,		"TST",	Iarith },	
  [OTEQ +CARITH2] =  { Idp2,		"TEQ",	Iarith },	
  [OCMP +CARITH2] =  { Idp2,		"CMP",	Iarith },	
  [OCMN +CARITH2] =  { Idp2,		"CMN",	Iarith },	
  [OORR +CARITH2] =  { Idp2,		"ORR",	Iarith },	
  [OMOV +CARITH2] =  { Idp2,		"MOV",	Iarith },	
  [OBIC +CARITH2] =  { Idp2,		"BIC",	Iarith },	
  [OMVN +CARITH2] =  { Idp2,		"MVN",	Iarith },	
  /*x: [[itab]] elements */
  //i,r,r
  [OAND +CARITH3] =  { Idp3,		"AND",	Iarith },	
  [OEOR +CARITH3] =  { Idp3,		"EOR",	Iarith },	
  [OSUB +CARITH3] =  { Idp3,		"SUB",	Iarith },	
  [ORSB +CARITH3] =  { Idp3,		"RSB",	Iarith },	
  [OADD +CARITH3] =  { Idp3,		"ADD",	Iarith },	
  [OADC +CARITH3] =  { Idp3,		"ADC",	Iarith },	
  [OSBC +CARITH3] =  { Idp3,		"SBC",	Iarith },	
  [ORSC +CARITH3] =  { Idp3,		"RSC",	Iarith },	
  [OTST +CARITH3] =  { Idp3,		"TST",	Iarith },	
  [OTEQ +CARITH3] =  { Idp3,		"TEQ",	Iarith },	
  [OCMP +CARITH3] =  { Idp3,		"CMP",	Iarith },	
  [OCMN +CARITH3] =  { Idp3,		"CMN",	Iarith },	
  [OORR +CARITH3] =  { Idp3,		"ORR",	Iarith },	
  [OMOV +CARITH3] =  { Idp3,		"MOV",	Iarith },	
  [OBIC +CARITH3] =  { Idp3,		"BIC",	Iarith },	
  [OMVN +CARITH3] =  { Idp3,		"MVN",	Iarith },	
  /*x: [[itab]] elements */
  [OMULLU]  =  { Imull,	"MULLU",	Iarith },
  [OMULALU] =  { Imull,	"MULALU",	Iarith },
  [OMULL]   =  { Imull,	"MULL",		Iarith },
  [OMULAL]  =  { Imull,	"MULAL",	Iarith },
  /*x: [[itab]] elements */
  [OSWPW] =   { Iswap,		"SWPW",	Imem },
  [OSWPBU] =  { Iswap,		"SWPBU",Imem },
  /*x: [[itab]] elements */
  // load/store w/ub i,r
  [OLDW +CMEM0] =  { Imem1,		"MOVW",	Imem },
  [OLDB +CMEM0] =  { Imem1,		"MOVB",	Imem },
  [OSTW +CMEM0] =  { Imem1,		"MOVW",	Imem },
  [OSTB +CMEM0] =  { Imem1,		"MOVB",	Imem },
  /*x: [[itab]] elements */
  // load/store r,r
  [OLDW +CMEM1] =  { Imem1,		"MOVW",	Imem },
  [OLDB +CMEM1] =  { Imem1,		"MOVB",	Imem },
  [OSTW +CMEM1] =  { Imem1,		"MOVW",	Imem },
  [OSTB +CMEM1] =  { Imem1,		"MOVB",	Imem },
  /*x: [[itab]] elements */
  // load/store h/sb
  [OLDH]  =  { Imem2,		"MOV",	Imem },
  [OLDBU] =  { Imem2,		"MOV",	Imem },
  [OSTH]  =  { Imem2,		"MOV",	Imem },
  [OSTBU] =  { Imem2,		"MOV",	Imem },
  /*x: [[itab]] elements */
  // block move r,r
  [OLDM] =  { Ilsm,		"LDM",	Imem },
  [OSTM] =  { Ilsm,		"STM",	Imem },
  /*x: [[itab]] elements */
  // branch
  [OB]  =  { Ib,	"B",	Ibranch },
  /*x: [[itab]] elements */
  [OBL] =  { Ibl,	"BL",	Ibranch },
  /*x: [[itab]] elements */
  [OSWI] =  { Ssyscall,		"SWI",	Isyscall },
  /*e: [[itab]] elements */
  { 0 }
};
/*e: global [[itab]] */


/*s: function [[runcmp]] */
bool
runcmp(void)
{
    switch(reg.instr_cond) {
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
        Bprint(bout, "unimplemented condition prefix %x (%ld %ld)\n",
            reg.instr_cond, reg.cc1, reg.cc2);
        undef(reg.instr);
        return false;
    }
}
/*e: function [[runcmp]] */

/*s: function [[runteq]] */
bool
runteq(void)
{
    long res = reg.cc1 ^ reg.cc2;

    switch(reg.instr_cond) {
    case 0x0:	/* eq */	return res == 0;
    case 0x1:	/* ne */	return res != 0;
    case 0x4:	/* mi */	return (res & SIGNBIT) != 0;
    case 0x5:	/* pl */	return (res & SIGNBIT) == 0;
    case 0xe:	/* al */	return true;
    case 0xf:	/* nv */	return false;
    default:
        Bprint(bout, "unimplemented condition prefix %x (%ld %ld)\n",
            reg.instr_cond, reg.cc1, reg.cc2);
        undef(reg.instr);
        return false;
    }
}
/*e: function [[runteq]] */

/*s: function [[runtst]] */
bool
runtst(void)
{
    long res = reg.cc1 & reg.cc2;

    switch(reg.instr_cond) {
    case 0x0:	/* eq */	return res == 0;
    case 0x1:	/* ne */	return res != 0;
    case 0x4:	/* mi */	return (res & SIGNBIT) != 0;
    case 0x5:	/* pl */	return (res & SIGNBIT) == 0;
    case 0xe:	/* al */	return true;
    case 0xf:	/* nv */	return false;
    default:
        Bprint(bout, "unimplemented condition prefix %x (%ld %ld)\n",
            reg.instr_cond, reg.cc1, reg.cc2);
        undef(reg.instr);
        return false;
    }
}
/*e: function [[runtst]] */

/*s: function [[run]] */
void
run(void)
{
    bool execute;

    do {
        reg.ar = reg.r[REGPC];
        reg.instr = ifetch(reg.ar);

        reg.instr_opcode = arm_class(reg.instr);
        reg.ip = &itab[reg.instr_opcode];

        /*s: [[run()]] set reg.cond */
        reg.instr_cond = (reg.instr>>28) & 0xf;
        /*e: [[run()]] set reg.cond */
        /*s: [[run()]] switch [[reg.compare_op]] to set [[execute]] */
        switch(reg.compare_op) {
        case CCcmp:
            execute = runcmp(); // use reg.instr_cond
            break;
        case CCteq:
            execute = runteq();
            break;
        case CCtst:
            execute = runtst();
            break;
        default:
            Bprint(bout, "unimplemented compare operation %x\n",
                reg.compare_op);
            return;
        }
        /*e: [[run()]] switch [[reg.compare_op]] to set [[execute]] */

        if(execute) {
            /*s: [[run()]] profile current instruction class */
            // profiling
            reg.ip->count++;
            /*e: [[run()]] profile current instruction class */
            // !!the dispatch!!
            (*reg.ip->func)(reg.instr);

        }
        else
          if(trace) itrace("%s%s IGNORED",reg.ip->name, cond[reg.instr_cond]);

        reg.r[REGPC] += 4; // simple archi with fixed-length instruction :)

        /*s: [[run()]] check for breakpoints */
        if(bplist)
            brkchk(reg.r[REGPC], Instruction);
        /*e: [[run()]] check for breakpoints */
    } while(--count);
}
/*e: function [[run]] */

/*s: function [[undef]] */
void
undef(instruction inst)
{
    Bprint(bout, "undefined instruction trap pc #%lux inst %.8lux op %d\n",
        reg.r[REGPC], inst, reg.instr_opcode);
    longjmp(errjmp, 0);
}
/*e: function [[undef]] */

/*s: function [[arm_class]] */
int
arm_class(instruction w)
{
    // between 0 and 7
    int class;
    // enum<opcode>
    int op;
    /*s: [[arm_class()]] locals */
    int x;
    /*e: [[arm_class()]] locals */
   
    class = (w >> 25) & 0x7;
    switch(class) {
    /*s: [[arm_class()]] class cases */
    case 0:	/* data processing r,r,r */
        x = ((w >> 4) & 0xf);
        /*s: [[arm_class()]] class 0, if x is 0x9 */
        /* mul, swp, mull */
        if(x == 0x9) {
            op = CMUL;
            /*s: [[arm_class()]] class 0, when x == 0x9, if bit 24 set */
            if(w & (1<<24)) {
                op = OSWPW;
                if(w & (1<<22))
                     op = OSWPBU;
                break;
            }
            /*e: [[arm_class()]] class 0, when x == 0x9, if bit 24 set */
            /*s: [[arm_class()]] class 0, when x == 0x9, if bit 23 set */
            if(w & (1<<23)) {	/* mullu */
                op = CMUL+2;
                if(w & (1<<22))	/* mull */
                    op = CMUL+4;
            }
            /*e: [[arm_class()]] class 0, when x == 0x9, if bit 23 set */
            if(w & (1<<21))
                op++;		/* mla */
            break;
        }
        /*e: [[arm_class()]] class 0, if x is 0x9 */
        /*s: [[arm_class()]] class 0, if x has 0x9 bits */
        if((x & 0x9) == 0x9) {		/* ld/st byte/half s/u */
             //                          OxxBU?               OSTx?
             op = CMEM_BASIS + CMEM2 + ((w >> 22) & 0x1) + ((w >> 19) & 0x2);
             break;
        }
        /*e: [[arm_class()]] class 0, if x has 0x9 bits */
        // else

        // the opcode! OAND/OADD/...
        op = (w >> 21) & 0xf;

        /*s: [[arm_class()]] class 0, adjust op for operands mode */
        if(w & (1<<4))
          op += CARITH2;
        else
         if((w & (31<<7)) || (w & (1<<5)))
          op += CARITH1;
        // else op += CARITH0
        /*e: [[arm_class()]] class 0, adjust op for operands mode */
        break;
    /*x: [[arm_class()]] class cases */
    case 1:	/* data processing i,r,r */
     op = CARITH3 + ((w >> 21) & 0xf);
     break;
    /*x: [[arm_class()]] class cases */
    case 2:	/* load/store byte/word i(r) */
     //                            OxxB?              OSTx?
     op = CMEM_BASIS + CMEM0 + ((w >> 22) & 0x1) + ((w >> 19) & 0x2);
     break;
    /*x: [[arm_class()]] class cases */
    case 3:	/* load/store byte/word (r)(r) */
     //                            OxxB?              OSTx?
     op = CMEM_BASIS + CMEM1 + ((w >> 22) & 0x1) + ((w >> 19) & 0x2);
     break;
    /*x: [[arm_class()]] class cases */
    case 4:	/* block data transfer (r)(r) */
     op = CBLOC + ((w >> 20) & 0x1);
     break;
    /*x: [[arm_class()]] class cases */
    case 5:	/* branch / branch link */
     op = CBRANCH + ((w >> 24) & 0x1);
     break;
    /*x: [[arm_class()]] class cases */
    case 7:	/* coprocessor crap */ // and syscall
     if((w >> 25) & 0x1)
       op = OSWI;
     else
       op = OUNDEF; // coprocessor stuff not handled
     break;
    /*e: [[arm_class()]] class cases */
    default:	  
        op = OUNDEF;
        break;
    }
    return op;
}
/*e: function [[arm_class]] */

/*s: function [[shift]] */
long
shift(long v, int st, int sc, bool isreg)
{
    /*s: [[shift()]] if sc is 0 */
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
    /*e: [[shift()]] if sc is 0 */
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
            v = (v << (32-sc)) 
                 | 
                ((ulong)v >> sc);
            break;
        }
    }
    return v;
}
/*e: function [[shift]] */

/*s: function [[dpex]] */
void
dpex(instruction inst, long o1, long o2, int rd)
{
    bool cbit = false;

    switch((inst>>21) & 0xf) {
    /*s: [[dpex()]] switch arith/logic opcode cases */
    case  OAND:
        reg.r[rd] = o1 & o2;
        cbit = true;
        break;
    case OORR:
        reg.r[rd] = o1 | o2;
        cbit = true;
        break;
    case OEOR:
        reg.r[rd] = o1 ^ o2;
        cbit = true;
        break;
    /*x: [[dpex()]] switch arith/logic opcode cases */
    case OBIC:
        reg.r[rd] = o1 & ~o2;
        cbit = true;
        break;
    /*x: [[dpex()]] switch arith/logic opcode cases */
    case OADD:
        /*s: [[dpex()]] if calltree, when add operation */
        if(calltree && rd == REGPC && o2 == 0) {
            Symbol s;

            findsym(o1 + o2, CTEXT, &s);
            Bprint(bout, "%8lux return to %lux %s r0=%lux\n",
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
    /*x: [[dpex()]] switch arith/logic opcode cases */
    case OCMN:
        if(inst & Sbit) { // not always true?
            reg.cc1 = o1;
            reg.cc2 = -o2;
            reg.compare_op = CCcmp;
        }
        return;
    /*x: [[dpex()]] switch arith/logic opcode cases */
    case  ORSB:
        reg.r[rd] = o2 - o1;
        if(inst & Sbit) {
            reg.cc1 = o2;
            reg.cc2 = o1;
            reg.compare_op = CCcmp;
        }
        return;
    /*x: [[dpex()]] switch arith/logic opcode cases */
    case  OADC:
    case  OSBC:
    case  ORSC:
        undef(inst);
    /*x: [[dpex()]] switch arith/logic opcode cases */
    case OMOV:
        reg.r[rd] = o2;
        cbit = true;
        break;
    case OMVN:
        reg.r[rd] = ~o2;
        cbit = true;
        break;
    /*x: [[dpex()]] switch arith/logic opcode cases */
    case OSUB:
        reg.r[rd] = o1 - o2;
        // Fallthrough
    case OCMP:
        if(inst & Sbit) {
            reg.cc1 = o1;
            reg.cc2 = o2;
            reg.compare_op = CCcmp;
        }
        return;
    /*x: [[dpex()]] switch arith/logic opcode cases */
    case  OTST:
        if(inst & Sbit) {
            reg.cc1 = o1;
            reg.cc2 = o2;
            reg.compare_op = CCtst;
        }
        return;
    case  OTEQ:
        if(inst & Sbit) { // not always true?
            reg.cc1 = o1;
            reg.cc2 = o2;
            reg.compare_op = CCteq;
        }
        return;
    /*e: [[dpex()]] switch arith/logic opcode cases */
    }
    /*s: [[dpex()]] if Sbit */
    if(inst & Sbit) {
        if(cbit)
            reg.cbit = reg.cout;
        reg.cc1 = reg.r[rd];
        reg.cc2 = 0;
        reg.compare_op = CCcmp;
    }
    /*e: [[dpex()]] if Sbit */
}
/*e: function [[dpex]] */

/*s: function [[Idp0]] */
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
    /*s: adjust o1 if rn is REGPC */
    if(rn == REGPC)
        o1 += 8;
    /*e: adjust o1 if rn is REGPC */
    o2 = reg.r[rm];
    /*s: adjust o2 if rm is REGPC */
    if(rm == REGPC)
        o2 += 8;
    /*e: adjust o2 if rm is REGPC */

    dpex(inst, o1, o2, rd);

    /*s: [[Idp0()]] trace */
    if(trace)
        itrace("%s%s\tR%d,R%d,R%d =#%x",
            reg.ip->name, cond[reg.instr_cond],
            rm, rn, rd,
            reg.r[rd]);
    /*e: [[Idp0()]] trace */
    /*s: [[Idpx()]] compensate REGPC */
    if(rd == REGPC)
        reg.r[rd] -= 4;
    /*e: [[Idpx()]] compensate REGPC */
}
/*e: function [[Idp0]] */

/*s: function [[Idp1]] */
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
    /*s: adjust o1 if rn is REGPC */
    if(rn == REGPC)
        o1 += 8;
    /*e: adjust o1 if rn is REGPC */
    o2 = reg.r[rm];
    /*s: adjust o2 if rm is REGPC */
    if(rm == REGPC)
        o2 += 8;
    /*e: adjust o2 if rm is REGPC */

    o2 = shift(o2, st, sc, false);
    dpex(inst, o1, o2, rd);

    /*s: [[Idp1()]] trace */
    if(trace)
        itrace("%s%s\tR%d%s%d,R%d,R%d =#%x",
            reg.ip->name, cond[reg.instr_cond], rm, shtype[st], sc, rn, rd,
            reg.r[rd]);
    /*e: [[Idp1()]] trace */
    /*s: [[Idpx()]] compensate REGPC */
    if(rd == REGPC)
        reg.r[rd] -= 4;
    /*e: [[Idpx()]] compensate REGPC */
}
/*e: function [[Idp1]] */

/*s: function [[Idp2]] */
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
    /*s: adjust o1 if rn is REGPC */
    if(rn == REGPC)
        o1 += 8;
    /*e: adjust o1 if rn is REGPC */
    o2 = reg.r[rm];
    /*s: adjust o2 if rm is REGPC */
    if(rm == REGPC)
        o2 += 8;
    /*e: adjust o2 if rm is REGPC */
    o3 = reg.r[rs];
    /*s: adjust o3 if rs is REGPC */
    if(rs == REGPC)
        o3 += 8;
    /*e: adjust o3 if rs is REGPC */

    o2 = shift(o2, st, o3, true);
    dpex(inst, o1, o2, rd);

    /*s: [[Idp2()]] trace */
    if(trace)
        itrace("%s%s\tR%d%sR%d=%d,R%d,R%d =#%x",
            reg.ip->name, cond[reg.instr_cond], rm, shtype[st], rs, o3, rn, rd,
            reg.r[rd]);
    /*e: [[Idp2()]] trace */
    /*s: [[Idpx()]] compensate REGPC */
    if(rd == REGPC)
        reg.r[rd] -= 4;
    /*e: [[Idpx()]] compensate REGPC */
}
/*e: function [[Idp2]] */

/*s: function [[Idp3]] */
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
    /*s: adjust o1 if rn is REGPC */
    if(rn == REGPC)
        o1 += 8;
    /*e: adjust o1 if rn is REGPC */

    o2 = inst & 0xff;
    sc = (inst>>7) & 0x1e;
    o2 = (o2 >> sc) | (o2 << (32 - sc)); // rotate

    dpex(inst, o1, o2, rd);

    /*s: [[Idp3()]] trace */
    if(trace)
        itrace("%s%s\t#%x,R%d,R%d =#%x",
            reg.ip->name, cond[reg.instr_cond], o2, rn, rd,
            reg.r[rd]);
    /*e: [[Idp3()]] trace */
    /*s: [[Idpx()]] compensate REGPC */
    if(rd == REGPC)
        reg.r[rd] -= 4;
    /*e: [[Idpx()]] compensate REGPC */
}
/*e: function [[Idp3]] */

/*s: function [[Imul]] */
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
            reg.ip->name, cond[reg.instr_cond], rs, rm, rd,
            reg.r[rd]);
    /*e: [[Imul()]] trace */
}
/*e: function [[Imul]] */

/*s: function [[Imull]] */
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

    if(inst & (1<<22)){ // mull
        v = (vlong)reg.r[rm] * (vlong)reg.r[rs];
        if(inst & (1 << 21)) // mull and accumulate
            v += reg.r[rn];
    }else{ // mullu
        v = XCAST(reg.r[rm]) * XCAST(reg.r[rs]);
        if(inst & (1 << 21)) // mullu and accumulate
            v += (ulong)reg.r[rn];
    }
    reg.r[rd] = v >> 32;
    reg.r[rn] = v;

    /*s: [[Imull()]] trace */
    if(trace)
        itrace("%s%s\tR%d,R%d,(R%d,R%d) =#%llx",
            reg.ip->name, cond[reg.instr_cond], rs, rm, rn, rd,
            v);
    /*e: [[Imull()]] trace */
}
/*e: function [[Imull]] */

/*s: function [[Imula]] */
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
            reg.ip->name, cond[reg.instr_cond], rs, rm, rn, rd,
            reg.r[rd]);
    /*e: [[Imula()]] trace */
}
/*e: function [[Imula]] */

/*s: function [[Iswap]] */
void
Iswap(instruction inst)
{
    int rn, rd, rm;
    ulong address, value;
    bool bbit;

    bbit = inst & (1<<22); // BU?

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

        bw = bbit? "B" : "";
        dotc = cond[reg.instr_cond];

        itrace("SWP%s%s\t#%x(R%d),R%d #%lux=#%x",
            bw, dotc,
            rn, rd,
            address, value);
    }
    /*e: [[Iswap()]] trace */
}
/*e: function [[Iswap]] */

/*s: function [[Imem1]] */
/*
 * load/store word/byte
 */
void
Imem1(instruction inst)
{
    int rn, rd, off, rm, sc, st;
    ulong address, value;
    bool prebit, ubit, bbit, wbit, lbit, bit25;

    bit25 = inst & (1<<25); // rm or I?
    prebit = inst & (1<<24); // Pre indexing?
    ubit = inst & (1<<23); // Up offset?
    bbit = inst & (1<<22); // Byte or Word?
    wbit = inst & (1<<21); // Write back address in rn?
    lbit = inst & (1<<20); // LDR or STR?

    rn = (inst>>16) & 0xf;
    rd = (inst>>12) & 0xf;

    SET(st);
    SET(sc);
    SET(rm);

    if(bit25) {
        // rm<>I(...)
        rm = inst & 0xf;
        st = (inst>>5) & 0x3;
        sc = (inst>>7) & 0x1f;
        off = reg.r[rm];
        if(rm == REGPC)
            off += 8;
        off = shift(off, st, sc, false);
    } else {
        // I(...)
        off = inst & 0xfff;
    }

    if(!ubit)
        off = -off;
    if(rn == REGPC)
        off += 8;

    address = reg.r[rn];
    if(prebit)
        address += off;

    if(lbit) {
        // LDR
        if(bbit)
            value = getmem_b(address);
        else
            value = getmem_w(address);
        if(rd == REGPC)
            value -= 4;
        reg.r[rd] = value;
    } else {
        // STR
        value = reg.r[rd];
        if(rd == REGPC)
            value -= 4;
        if(bbit)
            putmem_b(address, value);
        else
            putmem_w(address, value);
    }
    if(!prebit || wbit)
        reg.r[rn] += off;

    /*s: [[Imem1()]] trace */
    if(trace) {
        char *bw, *dotp, *dotc;

        bw = bbit ? "BU" : "W";
        dotp = prebit? "" : ".P";
        dotc = cond[reg.instr_cond];

        if(lbit) {
            // LDR
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
            // STR
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
/*e: function [[Imem1]] */

/*s: function [[Imem2]] */
/*
 * load/store unsigned byte/half word
 */
void
Imem2(instruction inst)
{
    int rn, rd, off, rm;
    ulong address, value;
    bool prebit, ubit, hbit, sbit, wbit, lbit, bit22;

    prebit = inst & (1<<24); // Pre indexing?
    ubit = inst & (1<<23); // Up offset?
    bit22 = inst & (1<<22);
    wbit = inst & (1<<21); // Write back address in rn
    lbit = inst & (1<<20); // LDR or STR?

    sbit = inst & (1<<6); // Signed?
    hbit = inst & (1<<5); // Half word or byte?

    rn = (inst>>16) & 0xf;
    rd = (inst>>12) & 0xf;

    SET(rm);
    if(bit22) {
        // I(...)
        off = ((inst>>4) & 0xf0) | (inst & 0xf);
    } else {
        // rm(...)
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
    if(prebit)
        address += off;

    if(lbit) {
        // LDR
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
        // STR
        value = reg.r[rd];
        if(rd == REGPC)
            value -= 4;
        if(hbit) {
            putmem_h(address, value);
        } else {
            putmem_b(address, value);
        }
    }
    if(!prebit || wbit)
        reg.r[rn] += off;

    /*s: [[Imem2()]] trace */
    if(trace) {
        char *hb, *dotp, *dotc;

        hb = hbit? "H" : "B";
        dotp = prebit? "" : ".P";
        dotc = cond[reg.instr_cond];

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
/*e: function [[Imem2]] */

/*s: function [[Ilsm]] */
void
Ilsm(instruction inst)
{
    bool prebit, ubit, sbit, wbit, lbit;
    int i, rn, reglist;
    ulong address, predelta, postdelta;

    prebit = (inst>>24) & 0x1;
    ubit = (inst>>23) & 0x1;
    sbit = (inst>>22) & 0x1;
    wbit = (inst>>21) & 0x1;
    lbit = (inst>>20) & 0x1;
    rn =   (inst>>16) & 0xf;
    reglist = inst & 0xffff;

    if(reglist & 0x8000)
        undef(reg.instr);
    if(sbit)
        undef(reg.instr);

    address = reg.r[rn];

    if(prebit) {
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
            (lbit ? "LDM" : "STM"), (ubit ? 'I' : 'D'), (prebit ? 'B' : 'A'),
            rn, reg.r[rn], (wbit ? "!" : ""), reglist);
    }
    /*e: [[Ilsm()]] trace */
}
/*e: function [[Ilsm]] */

/*s: function [[Ib]] */
void
Ib(instruction inst)
{
    long v;

    v = inst & 0xffffff; // 24 bits
    v = reg.r[REGPC] + (v << 2) + 8;
    /*s: [[Ib()]] trace */
    if(trace)
        itrace("B%s\t#%lux", cond[reg.instr_cond], v);
    /*e: [[Ib()]] trace */
    reg.r[REGPC] = v - 4;
}
/*e: function [[Ib]] */

/*s: function [[Ibl]] */
void
Ibl(instruction inst)
{
    long v;
    Symbol s;

    v = inst & 0xffffff;
    v = reg.r[REGPC] + (v << 2) + 8;
    /*s: [[Ibl()]] trace */
    if(trace)
        itrace("BL%s\t#%lux", cond[reg.instr_cond], v);
    /*e: [[Ibl()]] trace */
    /*s: [[Ibl()]] if calltree */
    if(calltree) {
        findsym(v, CTEXT, &s);
        Bprint(bout, "%8lux %s(", reg.r[REGPC], s.name);
        printparams(&s, reg.r[REGSP]);
        Bprint(bout, "from ");
        printsource(reg.r[REGPC]);
        Bputc(bout, '\n');
    }
    /*e: [[Ibl()]] if calltree */
    reg.r[REGLINK] = reg.r[REGPC] + 4;
    reg.r[REGPC] = v - 4;
}
/*e: function [[Ibl]] */
/*e: machine/5i/run.c */
