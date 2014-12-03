/*s: include/arm/5.out.h */

/*s: constant NSNAME(arm) */
#define	NSNAME		8
/*e: constant NSNAME(arm) */

/*s: constant NREG(arm) */
#define	NREG		16
/*e: constant NREG(arm) */

/*s: constant NOPROF(arm) */
#define NOPROF		(1<<0)
/*e: constant NOPROF(arm) */
/*s: constant DUPOK(arm) */
#define DUPOK		(1<<1)
/*e: constant DUPOK(arm) */

/*s: enum regxxx(arm) */
enum regxxx {
    REGRET =	0,
    REGARG =	0,

   /* compiler allocates R1 up as temps */
   /* compiler allocates register variables R2 up */
    REGMIN =	2,
    REGMAX =	8,

    REGEXT =	10,
   /* compiler allocates external registers R10 down */
    REGTMP =	11,

    REGSB =		12,
    REGSP =		13,
    REGLINK =	14,
    REGPC =		15,
};
/*e: enum regxxx(arm) */

/*s: constant NFREG(arm) */
#define	NFREG		8
/*e: constant NFREG(arm) */
/*s: constant FREGRET(arm) */
#define	FREGRET		0
/*e: constant FREGRET(arm) */
/*s: constant FREGEXT(arm) */
#define	FREGEXT		7
/*e: constant FREGEXT(arm) */
/*s: constant FREGTMP(arm) */
#define	FREGTMP		15
/*e: constant FREGTMP(arm) */

/* compiler allocates register variables F0 up */
/*s: enum as(arm) */
/* compiler allocates external registers F7 down */
// coupling: with 5c/enam.c
enum opcode
{
    AXXX,

    AAND,
    AEOR,
    ASUB,
    ARSB,
    AADD,
    AADC,
    ASBC,
    ARSC,
    ATST,
    ATEQ,
    ACMP,
    ACMN,
    AORR,
    //AMOV, redundant with the other MOV operations
    ABIC,
    AMVN,

    AB,
    ABL,
/* 
 * Do not reorder or fragment the conditional branch 
 * opcodes, or the predication code will break 
 */ 
    // AB derivatives with condition code, see 5i/
    ABEQ,
    ABNE,
    ABCS,//not in 5i/cond, seems equivalent to ABHS
    ABHS,
    ABCC,//not in 5i/cond, seems equivalent to ABLO
    ABLO,
    ABMI,
    ABPL,
    ABVS,
    ABVC,
    ABHI,
    ABLS,
    ABGE,
    ABLT,
    ABGT,
    ABLE,
    //ABAL? (always) done via AB, ABNV (never) done via ANOP probably

    // ??
    AMOVWD,
    AMOVWF,
    AMOVDW,
    AMOVFW,
    AMOVFD,
    AMOVDF,
    AMOVF,
    AMOVD,

    // floats?
    ACMPF,
    ACMPD,
    AADDF,
    AADDD,
    ASUBF,
    ASUBD,
    AMULF,
    AMULD,
    ADIVF,
    ADIVD,
//	ASQRTF, see below
//	ASQRTD, see below

    // pseudo instruction? just special kind of AMOV with shift bits?
    ASRL,
    ASRA,
    ASLL,

    // ARM has ADIV? AMOD?
    AMULU,
    ADIVU,
    AMUL,
    ADIV,
    AMOD,
    AMODU,

    AMOVB,
    AMOVBU,
    AMOVH,
    AMOVHU,
    AMOVW,
    AMOVM,

    ASWPBU,
    ASWPW,

    ANOP, // mv to beginning?

    ARFE, // ?? return from exn?
    ASWI, // syscall
    AMULA, // mov with MUL?

    // pseudo
    ADATA,
    AGLOBL,
    AGOK,
    AHISTORY,
    ANAME,
    ARET,
    ATEXT,
    AWORD,
    ADYNT,
    AINIT,
    ABCASE,
    ACASE,

    AEND, // hmm not really, who uses that?

    AMULL,
    AMULAL,
    AMULLU,
    AMULALU,

    ABX, // ?
    ABXRET, // ?
    ADWORD, // ?

    ASIGNAME,

    /* moved here to preserve values of older identifiers */
    ASQRTF,
    ASQRTD,

    ALDREX,
    ASTREX,
    
    ALDREXD,
    ASTREXD,

    ALAST,
};
/*e: enum as(arm) */

/*s: constant C_SCOND(arm) */
/* scond byte */
#define	C_SCOND	((1<<4)-1)
/*e: constant C_SCOND(arm) */
/*s: constant C_SBIT(arm) */
#define	C_SBIT	(1<<4)
/*e: constant C_SBIT(arm) */
/*s: constant C_PBIT(arm) */
#define	C_PBIT	(1<<5)
/*e: constant C_PBIT(arm) */
/*s: constant C_WBIT(arm) */
#define	C_WBIT	(1<<6)
/*e: constant C_WBIT(arm) */
/*s: constant C_FBIT(arm) */
#define	C_FBIT	(1<<7)	/* psr flags-only */
/*e: constant C_FBIT(arm) */
/*s: constant C_UBIT(arm) */
#define	C_UBIT	(1<<7)	/* up bit */
/*e: constant C_UBIT(arm) */

/*s: enum dxxx(arm) */
enum dxxx {
    D_GOK,

    D_NONE,
    // for B, BL?
    D_BRANCH,
    D_OREG,

    // For ADATA?
    D_EXTERN, // data/bss values (from SB)
    D_STATIC, // data static variables (from SB)
    D_AUTO, // stack values (from SP)
    D_PARAM, // parameter (from FP)

    D_CONST,
    D_FCONST,
    D_SCONST,

    D_PSR,
    D_XXX,
    D_REG,
    D_FREG,
    D_XXX2,
    D_XXX3,

    D_FILE,
    D_OCONST,
    D_FILE1, // used by linker only?
    D_SHIFT,
    D_FPCR,
    D_REGREG,

    D_ADDR,
};
/*e: enum dxxx(arm) */

/*s: constant SYMDEF(arm) */
/*
 * this is the ranlib header
 */
#define	SYMDEF	"__.SYMDEF"
/*e: constant SYMDEF(arm) */
/*e: include/arm/5.out.h */
