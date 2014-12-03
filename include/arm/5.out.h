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

/*s: constant ALLTHUMBS(arm) */
#define	ALLTHUMBS	(1<<2)
/*e: constant ALLTHUMBS(arm) */

/*s: constant REGRET(arm) */
#define	REGRET		0
/*e: constant REGRET(arm) */
/*s: constant REGARG(arm) */
#define	REGARG		0
/*e: constant REGARG(arm) */

/* compiler allocates R1 up as temps */
/*s: constant REGMIN(arm) */
/* compiler allocates register variables R2 up */
#define	REGMIN		2
/*e: constant REGMIN(arm) */
/*s: constant REGMAX(arm) */
#define	REGMAX		8
/*e: constant REGMAX(arm) */
/*s: constant REGEXT(arm) */
#define	REGEXT		10
/*e: constant REGEXT(arm) */
/*s: constant REGTMP(arm) */
/* compiler allocates external registers R10 down */
#define	REGTMP		11
/*e: constant REGTMP(arm) */
/*s: constant REGSB(arm) */
#define	REGSB		12
/*e: constant REGSB(arm) */
/*s: constant REGSP(arm) */
#define	REGSP		13
/*e: constant REGSP(arm) */
/*s: constant REGLINK(arm) */
#define	REGLINK		14
/*e: constant REGLINK(arm) */
/*s: constant REGPC(arm) */
#define	REGPC		15
/*e: constant REGPC(arm) */

/*s: constant REGTMPT(arm) */
#define	REGTMPT		7	/* used by the loader for thumb code */
/*e: constant REGTMPT(arm) */

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

enum	as
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

/*s: constant D_GOK(arm) */
/* type/name */
#define	D_GOK	0
/*e: constant D_GOK(arm) */
/*s: constant D_NONE(arm) */
#define	D_NONE	1
/*e: constant D_NONE(arm) */

/*s: constant D_BRANCH(arm) */
/* type */
#define	D_BRANCH	(D_NONE+1)
/*e: constant D_BRANCH(arm) */
/*s: constant D_OREG(arm) */
#define	D_OREG		(D_NONE+2)
/*e: constant D_OREG(arm) */

/*s: constant D_CONST(arm) */
#define	D_CONST		(D_NONE+7)
/*e: constant D_CONST(arm) */
/*s: constant D_FCONST(arm) */
#define	D_FCONST	(D_NONE+8)
/*e: constant D_FCONST(arm) */
/*s: constant D_SCONST(arm) */
#define	D_SCONST	(D_NONE+9)
/*e: constant D_SCONST(arm) */
/*s: constant D_PSR(arm) */
#define	D_PSR		(D_NONE+10)
/*e: constant D_PSR(arm) */
/*s: constant D_REG(arm) */
#define	D_REG		(D_NONE+12)
/*e: constant D_REG(arm) */
/*s: constant D_FREG(arm) */
#define	D_FREG		(D_NONE+13)
/*e: constant D_FREG(arm) */
/*s: constant D_FILE(arm) */
#define	D_FILE		(D_NONE+16)
/*e: constant D_FILE(arm) */
/*s: constant D_OCONST(arm) */
#define	D_OCONST	(D_NONE+17)
/*e: constant D_OCONST(arm) */
/*s: constant D_FILE1(arm) */
#define	D_FILE1		(D_NONE+18)
/*e: constant D_FILE1(arm) */

/*s: constant D_SHIFT(arm) */
#define	D_SHIFT		(D_NONE+19)
/*e: constant D_SHIFT(arm) */
/*s: constant D_FPCR(arm) */
#define	D_FPCR		(D_NONE+20)
/*e: constant D_FPCR(arm) */
/*s: constant D_REGREG(arm) */
#define	D_REGREG	(D_NONE+21)
/*e: constant D_REGREG(arm) */
/*s: constant D_ADDR(arm) */
#define	D_ADDR		(D_NONE+22)
/*e: constant D_ADDR(arm) */

/*s: constant D_EXTERN(arm) */
/* name */
#define	D_EXTERN	(D_NONE+3)
/*e: constant D_EXTERN(arm) */
/*s: constant D_STATIC(arm) */
#define	D_STATIC	(D_NONE+4)
/*e: constant D_STATIC(arm) */
/*s: constant D_AUTO(arm) */
#define	D_AUTO		(D_NONE+5)
/*e: constant D_AUTO(arm) */
/*s: constant D_PARAM(arm) */
#define	D_PARAM		(D_NONE+6)
/*e: constant D_PARAM(arm) */

/*s: constant SYMDEF(arm) */
/*
 * this is the ranlib header
 */
#define	SYMDEF	"__.SYMDEF"
/*e: constant SYMDEF(arm) */
/*e: include/arm/5.out.h */
