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
    REGSB =		12, // static base

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
    // ---------------------------------------------------------
    // Arithmetic and logic opcodes
    // ---------------------------------------------------------
    /*s: [[Opcode]] cases, logic opcodes */
    AAND,
    AORR,
    AEOR,
    /*x: [[Opcode]] cases, logic opcodes */
    ABIC,
    /*e: [[Opcode]] cases, logic opcodes */
    /*s: [[Opcode]] cases, add/sub opcodes */
    AADD,
    ASUB,
    /*x: [[Opcode]] cases, add/sub opcodes */
    ARSB,
    AADC,
    ASBC,
    ARSC,
    /*e: [[Opcode]] cases, add/sub opcodes */
    /*s: [[Opcode]] cases, mul/div/mod opcodes */
    AMUL,
    ADIV, // VIRTUAL, transformed to call to _div
    AMOD, // VIRTUAL, transformed to call to _mod
    /*x: [[Opcode]] cases, mul/div/mod opcodes */
    AMULL,
    AMULAL,
    AMULLU,
    AMULALU,
    /*x: [[Opcode]] cases, mul/div/mod opcodes */
    AMULA,
    /*x: [[Opcode]] cases, mul/div/mod opcodes */
    AMULU,
    ADIVU, // VIRTUAL, transformed to call to _divu
    AMODU, // VIRTUAL, transformed to call to _modu
    /*e: [[Opcode]] cases, mul/div/mod opcodes */
    /*s: [[Opcode]] cases, bitshift opcodes */
    ASLL,
    ASRL,
    /*x: [[Opcode]] cases, bitshift opcodes */
    ASRA,
    /*e: [[Opcode]] cases, bitshift opcodes */
    // works with the branching opcodes
    /*s: [[Opcode]] cases, comparison opcodes */
    ACMP,
    /*x: [[Opcode]] cases, comparison opcodes */
    ATST,
    ATEQ,
    /*x: [[Opcode]] cases, comparison opcodes */
    ACMN,
    /*e: [[Opcode]] cases, comparison opcodes */
    // ---------------------------------------------------------
    // Memory MOV opcodes
    // ---------------------------------------------------------
    /*s: [[Opcode]] cases, mov opcodes */
    AMOVW, // VIRTUAL, transformed in load and store instructions
    AMOVB,
    AMOVBU,
    AMOVH,
    AMOVHU,
    /*x: [[Opcode]] cases, mov opcodes */
    AMVN,
    /*x: [[Opcode]] cases, mov opcodes */
    AMOVM,
    /*e: [[Opcode]] cases, mov opcodes */
    /*s: [[Opcode]] cases, swap opcodes */
    ASWPW,
    ASWPBU,
    /*e: [[Opcode]] cases, swap opcodes */
    // ---------------------------------------------------------
    // Control flow opcodes
    // ---------------------------------------------------------
    /*s: [[Opcode]] cases, branching opcodes */
    AB,  // =~ JMP
    /*x: [[Opcode]] cases, branching opcodes */
    ABL, // =~ CALL, Branch and Link
    /*x: [[Opcode]] cases, branching opcodes */
    ARET, // VIRTUAL, transformed to B (R14) or MOV xxx(SP), R15
    /*x: [[Opcode]] cases, branching opcodes */
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
    /*e: [[Opcode]] cases, branching opcodes */
    // ---------------------------------------------------------
    // Syscall
    // ---------------------------------------------------------
    /*s: [[Opcode]] cases, interrupt opcodes */
    ASWI, // syscall
    /*x: [[Opcode]] cases, interrupt opcodes */
    ARFE, // VIRTUAL, return from exception/interrupt, MOVM.IA.S.W (R13), [R15]
    /*e: [[Opcode]] cases, interrupt opcodes */
    // ---------------------------------------------------------
    // Float opcodes
    // ---------------------------------------------------------
    /*s: [[Opcode]] cases, float mov opcodes */
    AMOVWD,
    AMOVWF,
    AMOVDW,
    AMOVFW,
    AMOVFD,
    AMOVDF,
    AMOVF,
    AMOVD,
    /*e: [[Opcode]] cases, float mov opcodes */
    /*s: [[Opcode]] cases, float arithmetic opcodes */
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
    /*e: [[Opcode]] cases, float arithmetic opcodes */
    // ---------------------------------------------------------
    // Pseudo opcodes
    // ---------------------------------------------------------
    /*s: [[Opcode]] cases, pseudo opcodes */
    ATEXT,
    AGLOBL,
    /*x: [[Opcode]] cases, pseudo opcodes */
    ADATA,
    AWORD,
    /*x: [[Opcode]] cases, pseudo opcodes */
    AEND,
    /*x: [[Opcode]] cases, pseudo opcodes */
    ANAME,
    /*x: [[Opcode]] cases, pseudo opcodes */
    AHISTORY,
    /*x: [[Opcode]] cases, pseudo opcodes */
    ACASE,
    ABCASE,
    /*x: [[Opcode]] cases, pseudo opcodes */
    ASIGNAME,
    /*x: [[Opcode]] cases, pseudo opcodes */
    AGOK,
    /*x: [[Opcode]] cases, pseudo opcodes */
    ADYNT,
    AINIT,
    /*e: [[Opcode]] cases, pseudo opcodes */

    ALAST,
};
/*e: enum opcode(arm) */

/*s: enum operand_kind(arm) */
enum operand_kind {
    D_NONE,

    D_CONST,
    D_SCONST,
    D_FCONST,

    D_REG,
    /*s: operand_kind cases */
    D_OREG,
    /*x: operand_kind cases */
    D_SHIFT,
    /*x: operand_kind cases */
    D_BRANCH,
    /*x: operand_kind cases */
    D_FREG,
    D_FPCR,
    /*x: operand_kind cases */
    D_REGREG,
    /*x: operand_kind cases */
    D_PSR,
    /*e: operand_kind cases */
};
/*e: enum operand_kind(arm) */

/*s: enum sym_kind(arm) */
enum sym_kind {
    N_NONE,

    D_EXTERN, // text/data/bss values (from SB)
    D_AUTO,   // stack values (from SP)
    D_PARAM,  // parameter (from FP)
    /*s: sym_kind cases */
    D_STATIC, // data static variables (from SB)
    /*x: sym_kind cases */
    D_FILE,
    /*x: sym_kind cases */
    D_FILE1, // used by linker only?
    /*e: sym_kind cases */
};
/*e: enum sym_kind(arm) */

/*s: constant NSNAME */
#define	NSNAME		8
/*e: constant NSNAME */

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
