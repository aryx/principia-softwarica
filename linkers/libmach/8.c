/*s: linkers/libmach/8.c */
/*
 * 386 definition
 */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include "/386/include/ureg.h"
#include <mach.h>

/*s: function REGOFF(x86) */
#define	REGOFF(x)	(ulong)(&((struct Ureg *) 0)->x)
/*e: function REGOFF(x86) */

/*s: constant PC(x86) */
#define PC		REGOFF(pc)
/*e: constant PC(x86) */
/*s: constant SP(x86) */
#define SP		REGOFF(sp)
/*e: constant SP(x86) */
/*s: constant AX(x86) */
#define	AX		REGOFF(ax)
/*e: constant AX(x86) */

/*s: constant REGSIZE(x86) */
#define	REGSIZE		sizeof(struct Ureg)
/*e: constant REGSIZE(x86) */
/*s: function FP_CTL(x86) */
#define FP_CTL(x)	(REGSIZE+4*(x))
/*e: function FP_CTL(x86) */
/*s: function FP_REG(x86) */
#define FP_REG(x)	(FP_CTL(7)+10*(x))
/*e: function FP_REG(x86) */
/*s: constant FPREGSIZE(x86) */
#define	FPREGSIZE	(7*4+8*10)
/*e: constant FPREGSIZE(x86) */

/*s: global i386reglist */
Reglist i386reglist[] = {
    {"DI",		REGOFF(di),	RINT, 'X'},
    {"SI",		REGOFF(si),	RINT, 'X'},
    {"BP",		REGOFF(bp),	RINT, 'X'},
    {"BX",		REGOFF(bx),	RINT, 'X'},
    {"DX",		REGOFF(dx),	RINT, 'X'},
    {"CX",		REGOFF(cx),	RINT, 'X'},
    {"AX",		REGOFF(ax),	RINT, 'X'},
    {"GS",		REGOFF(gs),	RINT, 'X'},
    {"FS",		REGOFF(fs),	RINT, 'X'},
    {"ES",		REGOFF(es),	RINT, 'X'},
    {"DS",		REGOFF(ds),	RINT, 'X'},
    {"TRAP",	REGOFF(trap), 	RINT, 'X'},
    {"ECODE",	REGOFF(ecode),	RINT, 'X'},
    {"PC",		PC,		RINT, 'X'},
    {"CS",		REGOFF(cs),	RINT, 'X'},
    {"EFLAGS",	REGOFF(flags),	RINT, 'X'},
    {"SP",		SP,		RINT, 'X'},
    {"SS",		REGOFF(ss),	RINT, 'X'},

    {"E0",		FP_CTL(0),	RFLT, 'X'},
    {"E1",		FP_CTL(1),	RFLT, 'X'},
    {"E2",		FP_CTL(2),	RFLT, 'X'},
    {"E3",		FP_CTL(3),	RFLT, 'X'},
    {"E4",		FP_CTL(4),	RFLT, 'X'},
    {"E5",		FP_CTL(5),	RFLT, 'X'},
    {"E6",		FP_CTL(6),	RFLT, 'X'},
    {"F0",		FP_REG(0),	RFLT, '3'},
    {"F1",		FP_REG(1),	RFLT, '3'},
    {"F2",		FP_REG(2),	RFLT, '3'},
    {"F3",		FP_REG(3),	RFLT, '3'},
    {"F4",		FP_REG(4),	RFLT, '3'},
    {"F5",		FP_REG(5),	RFLT, '3'},
    {"F6",		FP_REG(6),	RFLT, '3'},
    {"F7",		FP_REG(7),	RFLT, '3'},
    {  0 }
};
/*e: global i386reglist */

/*s: global mi386 */
Mach mi386 =
{
    "386",
    MI386,		/* machine type */
    i386reglist,	/* register list */
    REGSIZE,	/* size of registers in bytes */
    FPREGSIZE,	/* size of fp registers in bytes */
    "PC",		/* name of PC */
    "SP",		/* name of SP */
    0,		/* link register */
    "setSB",	/* static base register name (bogus anyways) */
    0,		/* static base register value */
    0x1000,		/* page size */
    0xF0100000ULL,	/* kernel base */
    0xF0000000ULL,	/* kernel text mask */
    0x7FFFFFFFULL,	/* user stack top */
    1,		/* quantization of pc */
    4,		/* szaddr */
    4,		/* szreg */
    4,		/* szfloat */
    8,		/* szdouble */
};
/*e: global mi386 */
/*e: linkers/libmach/8.c */
