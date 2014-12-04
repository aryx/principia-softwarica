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
/* compiler allocates external registers F7 down */

/*s: enum as(arm) */
// coupling: with 5c/enam.c
enum opcode
{
    AXXX,

    ANOP,
    /*s: logic opcodes */
    AAND,
    AORR,
    AEOR,
    /*x: logic opcodes */
    ABIC,
    /*e: logic opcodes */
    /*s: add/sub opcodes */
    AADD,
    ASUB,
    /*x: add/sub opcodes */
    ARSB,
    AADC,
    ASBC,
    ARSC,
    /*e: add/sub opcodes */
    /*s: mul/div/mod opcodes */
    AMUL,
    ADIV,
    AMOD,
    /*x: mul/div/mod opcodes */
    AMULU,
    ADIVU,
    AMODU,
    /*x: mul/div/mod opcodes */
    AMULL,
    AMULAL,
    AMULLU,
    AMULALU,
    /*x: mul/div/mod opcodes */
    AMULA, // mov with MUL?
    /*e: mul/div/mod opcodes */
    /*s: comparison opcodes */
    ATST,
    ATEQ,
    ACMP,
    /*x: comparison opcodes */
    ACMN,
    /*e: comparison opcodes */
    /*s: bitshift opcodes */
    // pseudo instruction? just special kind of AMOV with shift bits?
    ASRL,
    ASRA,
    ASLL,
    /*e: bitshift opcodes */

    /*s: branching opcodes */
    AB,
    ABL,
    /*x: branching opcodes */
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
    /*e: branching opcodes */

    /*s: mov opcodes */
    AMOVW,
    /*x: mov opcodes */
    AMOVB,
    AMOVBU,
    AMOVH,
    AMOVHU,
    /*x: mov opcodes */
    AMVN,
    /*x: mov opcodes */
    AMOVM,
    /*e: mov opcodes */
    /*s: swap opcodes */
    ASWPW,
    ASWPBU,
    /*e: swap opcodes */

    /*s: syscall opcodes */
    ASWI, // syscall
    /*e: syscall opcodes */

    /*s: pseudo opcodes */
    ARET,
    /*x: pseudo opcodes */
    AGOK,
    AHISTORY,
    ANAME,
    ADYNT,
    AINIT,
    ABCASE,
    /*x: pseudo opcodes */
    ATEXT,
    AGLOBL,
    /*x: pseudo opcodes */
    ADATA,
    AWORD,
    /*x: pseudo opcodes */
    AEND,
    /*x: pseudo opcodes */
    ACASE,
    /*e: pseudo opcodes */

    /*s: mov float opcodes */
    // ??
    AMOVWD,
    AMOVWF,
    AMOVDW,
    AMOVFW,
    AMOVFD,
    AMOVDF,
    AMOVF,
    AMOVD,
    /*e: mov float opcodes */
    /*s: arithmetic float opcodes */
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
    ASQRTF,
    ASQRTD,
    /*e: arithmetic float opcodes */
    /*s: misc opcodes */
    ARFE, // ?? return from exn?
    ABX, // ?
    ABXRET, // ?
    ADWORD, // ?
    ASIGNAME,
    /* moved here to preserve values of older identifiers */
    ALDREX,
    ASTREX,
    ALDREXD,
    ASTREXD,
    /*e: misc opcodes */

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
enum operand_kind {
    D_GOK,

    D_NONE,

    D_REG,
    D_CONST,
    D_SHIFT,

    // for B, BL
    D_BRANCH,

    // For ADATA?
    D_EXTERN, // data/bss values (from SB)
    D_STATIC, // data static variables (from SB)
    D_AUTO,   // stack values (from SP)
    D_PARAM,  // parameter (from FP)

    D_FCONST,
    D_SCONST,

    D_OREG,
    D_OCONST,

    D_PSR,
    D_FPCR,
    D_FREG,
    D_REGREG,

    D_FILE,

    D_FILE1, // used by linker only?
    D_ADDR, // used by linker only?

};
/*e: enum dxxx(arm) */

/*s: constant SYMDEF(arm) */
/*
 * this is the ranlib header
 */
#define	SYMDEF	"__.SYMDEF"
/*e: constant SYMDEF(arm) */
/*e: include/arm/5.out.h */
