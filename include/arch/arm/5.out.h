/*s: include/arm/5.out.h */

// Many of the types below are marshalled in the .5 object files
// so take care when changing those types to not alter the order
// (or to recompile carefully everything).

/*s: enum regxxx(arm) */
enum registr {
    /*s: [[regxxx]] compiler conventions cases */
    REGRET =	0,
    REGARG =	0,
    /* compiler allocates R1 up as temps */
    /* compiler allocates register variables R2 up */
    REGMIN =	2,
    REGMAX =	8,
    REGEXT =	10, // R9/R10 possible 'extern register xxx;'
    /* compiler allocates external registers R10 down */
    REGTMP =	11,
    /*e: [[regxxx]] compiler conventions cases */
    REGSB =		12, // SB? segment base?

    REGSP =		13,
    REGLINK =	14,
    REGPC =		15,

    NREG = 16,
};
/*e: enum regxxx(arm) */
/*s: constant R_NONE(arm) */
#define R_NONE 16
/*e: constant R_NONE(arm) */

/*s: enum fregister(arm) */
enum fregister {
    FREGRET = 0,
    /* compiler allocates register variables F0 up */
    FREGEXT = 7,
    /* compiler allocates external registers F7 down */
    FREGTMP = 15, // ??

    NFREG = 8,
};
/*e: enum fregister(arm) */

/*s: enum opcode(arm) */
// coupling: with 5c/enam.c
enum opcode
{
    AXXX,

    ANOP, // VIRTUAL removed by linker
    // ----------------------------------------------------------------------
    // Arithmetic and logic opcodes
    // ----------------------------------------------------------------------
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
    ADIV, // VIRTUAL, transformed to call to _div
    AMOD, // VIRTUAL, transformed to call to _mod
    /*x: mul/div/mod opcodes */
    AMULL,
    AMULAL,
    AMULLU,
    AMULALU,
    /*x: mul/div/mod opcodes */
    AMULA,
    /*x: mul/div/mod opcodes */
    AMULU,
    ADIVU, // VIRTUAL, transformed to call to _divu
    AMODU, // VIRTUAL, transformed to call to _modu
    /*e: mul/div/mod opcodes */
    /*s: bitshift opcodes */
    ASRL,
    ASRA,
    ASLL,
    /*e: bitshift opcodes */
    /*s: comparison opcodes */
    ATST,
    ATEQ,
    ACMP,
    /*x: comparison opcodes */
    ACMN,
    /*e: comparison opcodes */
    // ----------------------------------------------------------------------
    // Branching opcodes
    // ----------------------------------------------------------------------
    /*s: branching opcodes */
    AB,
    ABL, // branch and link, =~ CALL
    /*x: branching opcodes */
    /* 
     * Do not reorder or fragment the conditional branch 
     * opcodes, or the predication code will break 
     */ 
    // VIRTUAL, AB derivatives with condition code, see 5i/
    ABEQ,
    ABNE,
    ABHS,
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
    /*x: branching opcodes */
    ARET, // VIRTUAL, transformed to B (R14) or MOV xxx(SP), R15
    /*e: branching opcodes */
    // ----------------------------------------------------------------------
    // Memory MOV opcodes
    // ----------------------------------------------------------------------
    /*s: mov opcodes */
    AMOVW, // VIRTUAL, transformed in load and store instructions
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
    // ----------------------------------------------------------------------
    // Syscall
    // ----------------------------------------------------------------------
    /*s: interrupt opcodes */
    ASWI, // syscall
    /*x: interrupt opcodes */
    ARFE, // VIRTUAL, return from exception/interrupt, MOVM.IA.S.W (R13), [R15]
    /*e: interrupt opcodes */
    // ----------------------------------------------------------------------
    // Float opcodes
    // ----------------------------------------------------------------------
    /*s: float mov opcodes */
    AMOVWD,
    AMOVWF,
    AMOVDW,
    AMOVFW,
    AMOVFD,
    AMOVDF,
    AMOVF,
    AMOVD,
    /*e: float mov opcodes */
    /*s: float arithmetic opcodes */
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
    /*e: float arithmetic opcodes */
    // ----------------------------------------------------------------------
    // Pseudo opcodes
    // ----------------------------------------------------------------------
    /*s: pseudo opcodes */
    ATEXT,
    AGLOBL,
    /*x: pseudo opcodes */
    ADATA,
    AWORD,
    /*x: pseudo opcodes */
    AEND,
    /*x: pseudo opcodes */
    ANAME,
    /*x: pseudo opcodes */
    AHISTORY,
    /*x: pseudo opcodes */
    ACASE,
    ABCASE,
    /*x: pseudo opcodes */
    ASIGNAME,
    /*x: pseudo opcodes */
    AGOK,
    /*x: pseudo opcodes */
    ADYNT,
    AINIT,
    /*e: pseudo opcodes */

    ALAST,
};
/*e: enum opcode(arm) */

/*s: enum operand_kind(arm) */
enum operand_kind {
    D_NONE,
    /*s: operand_kind cases */
    D_REG,
    /*x: operand_kind cases */
    D_CONST,
    /*x: operand_kind cases */
    D_SHIFT,
    /*x: operand_kind cases */
    D_OREG,
    /*x: operand_kind cases */
    D_BRANCH,
    /*x: operand_kind cases */
    D_PSR,
    /*x: operand_kind cases */
    D_SCONST,
    /*x: operand_kind cases */
    D_REGREG,
    /*x: operand_kind cases */
    D_FREG,
    D_FCONST,
    D_FPCR,
    /*e: operand_kind cases */
};
/*e: enum operand_kind(arm) */

/*s: enum sym_kind(arm) */
enum sym_kind {
   N_NONE,
   /*s: sym_kind cases */
   D_EXTERN, // text/data/bss values (from SB)
   D_AUTO,   // stack values (from SP)
   D_PARAM,  // parameter (from FP)
   /*x: sym_kind cases */
   D_STATIC, // data static variables (from SB)
   /*x: sym_kind cases */
   D_FILE,
   /*x: sym_kind cases */
   D_FILE1, // used by linker only?
   /*e: sym_kind cases */
};
/*e: enum sym_kind(arm) */

/*s: constant NSNAME(arm) */
#define	NSNAME		8
/*e: constant NSNAME(arm) */

// Attributes
/*s: constant NOPROF(arm) */
#define NOPROF		(1<<0)
/*e: constant NOPROF(arm) */
/*s: constant DUPOK(arm) */
#define DUPOK		(1<<1)
/*e: constant DUPOK(arm) */
//other attributes?
//old: #define	ALLTHUMBS	(1<<2)

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

#define COND_ALWAYS 14

/*s: constant SYMDEF(arm) */
/*
 * this is the ranlib header
 */
#define	SYMDEF	"__.SYMDEF"
/*e: constant SYMDEF(arm) */
/*e: include/arm/5.out.h */
