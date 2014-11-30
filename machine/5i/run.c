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
  //r,r,r
  [ 0] =  { Idp0,		"AND",	Iarith },	
  [ 1] =  { Idp0,		"EOR",	Iarith },	
  [ 2] =  { Idp0,		"SUB",	Iarith },	
  [ 3] =  { Idp0,		"RSB",	Iarith },	
  [ 4] =  { Idp0,		"ADD",	Iarith },	
  [ 5] =  { Idp0,		"ADC",	Iarith },	
  [ 6] =  { Idp0,		"SBC",	Iarith },	
  [ 7] =  { Idp0,		"RSC",	Iarith },	
  [ 8] =  { Idp0,		"TST",	Iarith },	
  [ 9] =  { Idp0,		"TEQ",	Iarith },	
  [10] =  { Idp0,		"CMP",	Iarith },	
  [11] =  { Idp0,		"CMN",	Iarith },	
  [12] =  { Idp0,		"ORR",	Iarith },	
  [13] =  { Idp0,		"MOV",	Iarith },	
  [14] =  { Idp0,		"BIC",	Iarith },	
  [15] =  { Idp0,		"MVN",	Iarith },	

  [16] =  { Idp1,		"AND",	Iarith },	
  [17] =  { Idp1,		"EOR",	Iarith },	
  [18] =  { Idp1,		"SUB",	Iarith },	
  [19] =  { Idp1,		"RSB",	Iarith },	
  [20] =  { Idp1,		"ADD",	Iarith },	
  [21] =  { Idp1,		"ADC",	Iarith },	
  [22] =  { Idp1,		"SBC",	Iarith },	
  [23] =  { Idp1,		"RSC",	Iarith },	
  [24] =  { Idp1,		"TST",	Iarith },	
  [25] =  { Idp1,		"TEQ",	Iarith },	
  [26] =  { Idp1,		"CMP",	Iarith },	
  [27] =  { Idp1,		"CMN",	Iarith },	
  [28] =  { Idp1,		"ORR",	Iarith },	
  [29] =  { Idp1,		"MOV",	Iarith },	
  [30] =  { Idp1,		"BIC",	Iarith },	
  [31] =  { Idp1,		"MVN",	Iarith },	

  [32] =  { Idp2,		"AND",	Iarith },	
  [33] =  { Idp2,		"EOR",	Iarith },	
  [34] =  { Idp2,		"SUB",	Iarith },	
  [35] =  { Idp2,		"RSB",	Iarith },	
  [36] =  { Idp2,		"ADD",	Iarith },	
  [37] =  { Idp2,		"ADC",	Iarith },	
  [38] =  { Idp2,		"SBC",	Iarith },	
  [39] =  { Idp2,		"RSC",	Iarith },	
  [40] =  { Idp2,		"TST",	Iarith },	
  [41] =  { Idp2,		"TEQ",	Iarith },	
  [42] =  { Idp2,		"CMP",	Iarith },	
  [43] =  { Idp2,		"CMN",	Iarith },	
  [44] =  { Idp2,		"ORR",	Iarith },	
  [45] =  { Idp2,		"MOV",	Iarith },	
  [46] =  { Idp2,		"BIC",	Iarith },	
  [47] =  { Idp2,		"MVN",	Iarith },	

  //i,r,r
  [48] =  { Idp3,		"AND",	Iarith },	
  [49] =  { Idp3,		"EOR",	Iarith },	
  [50] =  { Idp3,		"SUB",	Iarith },	
  [51] =  { Idp3,		"RSB",	Iarith },	
  [52] =  { Idp3,		"ADD",	Iarith },	
  [53] =  { Idp3,		"ADC",	Iarith },	
  [54] =  { Idp3,		"SBC",	Iarith },	
  [55] =  { Idp3,		"RSC",	Iarith },	
  [56] =  { Idp3,		"TST",	Iarith },	
  [57] =  { Idp3,		"TEQ",	Iarith },	
  [58] =  { Idp3,		"CMP",	Iarith },	
  [59] =  { Idp3,		"CMN",	Iarith },	
  [60] =  { Idp3,		"ORR",	Iarith },	
  [61] =  { Idp3,		"MOV",	Iarith },	
  [62] =  { Idp3,		"BIC",	Iarith },	
  [63] =  { Idp3,		"MVN",	Iarith },	

  [64] =  { Imul,	"MUL",	Iarith },
  [65] =  { Imula,	"MULA",	Iarith },	

  [66] =  { Iswap,		"SWPW",	Imem },
  [67] =  { Iswap,		"SWPBU",Imem },

  // load/store h/sb
  [68] =  { Imem2,		"MOV",	Imem },
  [69] =  { Imem2,		"MOV",	Imem },
  [70] =  { Imem2,		"MOV",	Imem },
  [71] =  { Imem2,		"MOV",	Imem },

  // load/store w/ub i,r
  [72] =  { Imem1,		"MOVW",	Imem },
  [73] =  { Imem1,		"MOVB",	Imem },
  [74] =  { Imem1,		"MOVW",	Imem },
  [75] =  { Imem1,		"MOVB",	Imem },
  // load/store r,r
  [76] =  { Imem1,		"MOVW",	Imem },
  [77] =  { Imem1,		"MOVB",	Imem },
  [78] =  { Imem1,		"MOVW",	Imem },
  [79] =  { Imem1,		"MOVB",	Imem },

  // block move r,r
  [80] =  { Ilsm,		"LDM",	Imem },
  [81] =  { Ilsm,		"STM",	Imem },

  // branch
  [82] =  { Ib,		"B",	Ibranch },
  [83] =  { Ibl,	"BL",	Ibranch },

  // co processor
  [84] =  { Ssyscall,		"SWI",	Isyscall },

  [85] =  { undef,		"undef" },
  [86] =  { undef,		"undef" },
  [87] =  { undef,		"undef" },

  [88] =  { Imull,		"MULLU",	Iarith },
  [89] =  { Imull,		"MULALU",	Iarith },
  [90] =  { Imull,		"MULL",		Iarith  },
  [91] =  { Imull,		"MULAL",	Iarith  },

  [92] =  { undef,		"undef"  },

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
 int op, done, cp;

 op = (w >> 25) & 0x7;
 switch(op) {
 case 0:	/* data processing r,r,r */
  if((w & 0x0ff00080) == 0x01200000) {
   op = (w >> 4) & 0x7;
   if(op == 7)
    op = 124;	/* bkpt */
   else if (op > 0 && op < 4)
    op += 124;	/* bx, blx */
   else
    op = 92;	/* unk */
   break;
  }
  op = ((w >> 4) & 0xf);
  if(op == 0x9) {
   op = 48+16;		/* mul, swp or *rex */
   if((w & 0x0ff00fff) == 0x01900f9f) {
    op = 93;	/* ldrex */
    break;
   }
   if((w & 0x0ff00ff0) == 0x01800f90) {
    op = 94;	/* strex */
    break;
   }
   if(w & (1<<24)) {
    op += 2;
    if(w & (1<<22))
     op++;	/* swpb */
    break;
   }
   if(w & (1<<23)) {	/* mullu */
    op = (48+24+4+4+2+2+4);
    if(w & (1<<22))	/* mull */
     op += 2;
   }
   if(w & (1<<21))
    op++;		/* mla */
   break;
  }
  if((op & 0x9) == 0x9)		/* ld/st byte/half s/u */
  {
   op = (48+16+4) + ((w >> 22) & 0x1) + ((w >> 19) & 0x2);
   break;
  }
  op = (w >> 21) & 0xf;
  if(w & (1<<4))
   op += 32;
  else
  if((w & (31<<7)) || (w & (1<<5)))
   op += 16;
  break;
 case 1:	/* data processing i,r,r */
  op = (48) + ((w >> 21) & 0xf);
  break;
 case 2:	/* load/store byte/word i(r) */
  if ((w & 0xffffff8f) == 0xf57ff00f) {	/* barriers, clrex */
   done = 1;
   switch ((w >> 4) & 7) {
   case 1:
    op = 95;	/* clrex */
    break;
   case 4:
    op = 96;	/* dsb */
    break;
   case 5:
    op = 97;	/* dmb */
    break;
   case 6:
    op = 98;	/* isb */
    break;
   default:
    done = 0;
    break;
   }
   if (done)
    break;
  }
  op = (48+24) + ((w >> 22) & 0x1) + ((w >> 19) & 0x2);
  break;
 case 3:	/* load/store byte/word (r)(r) */
  op = (48+24+4) + ((w >> 22) & 0x1) + ((w >> 19) & 0x2);
  break;
 case 4:	/* block data transfer (r)(r) */
  if ((w & 0xfe50ffff) == 0xf8100a00) {	/* v7 RFE */
   op = 99;
   break;
  }
  op = (48+24+4+4) + ((w >> 20) & 0x1);
  break;
 case 5:	/* branch / branch link */
  op = (48+24+4+4+2) + ((w >> 24) & 0x1);
  break;
 case 7:	/* coprocessor crap */
  cp = (w >> 8) & 0xF;
  if(cp == 10 || cp == 11){	/* vfp */
   if((w >> 4) & 0x1){
    /* vfp register transfer */
    switch((w >> 21) & 0x7){
    case 0:
     op = 118 + ((w >> 20) & 0x1);
     break;
    case 7:
     op = 118+2 + ((w >> 20) & 0x1);
     break;
    default:
     op = (48+24+4+4+2+2+4+4);
     break;
    }
    break;
   }
   /* vfp data processing */
   if(((w >> 23) & 0x1) == 0){
    op = 100 + ((w >> 19) & 0x6) + ((w >> 6) & 0x1);
    break;
   }
   switch(((w >> 19) & 0x6) + ((w >> 6) & 0x1)){
   case 0:
    op = 108;
    break;
   case 7:
    if(((w >> 19) & 0x1) == 0){
     if(((w >> 17) & 0x1) == 0)
      op = 109 + ((w >> 16) & 0x4) +
       ((w >> 15) & 0x2) +
       ((w >> 7) & 0x1);
     else if(((w >> 16) & 0x7) == 0x7)
      op = 117;
    }else
     switch((w >> 16) & 0x7){
     case 0:
     case 4:
     case 5:
      op = 117;
      break;
     }
    break;
   }
   if(op == 7)
    op = (48+24+4+4+2+2+4+4);
   break;
  }
  op = (48+24+4+4+2+2) + ((w >> 3) & 0x2) + ((w >> 20) & 0x1);
  break;
 case 6:	/* vfp load / store */
  if(((w >> 21) &0x9) == 0x8){
   op = 122 + ((w >> 20) & 0x1);
   break;
  }
  /* fall through */
 default:	  
  op = (48+24+4+4+2+2+4+4);
  break;
 }
 return op;
}
/*e: function arm_class */

/*s: function shift */
long
shift(long v, int st, int sc, int isreg)
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
    o2 = shift(o2, st, sc, 0);

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
    o2 = shift(o2, st, o3, 1);

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
        off = shift(off, st, sc, 0);
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
