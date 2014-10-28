/*s: include/386/8.out.h */

/*s: constant NSNAME */
#define	NSNAME	8
/*e: constant NSNAME */
/*s: constant NOPROF */
#define NOPROF	(1<<0)
/*e: constant NOPROF */
/*s: constant DUPOK */
#define DUPOK	(1<<1)
/*e: constant DUPOK */
/*s: constant NOSPLIT */
#define NOSPLIT	(1<<2)
/*e: constant NOSPLIT */

/*s: enum as */
// coupling: with 8c/enam.c, 8l/obj.c#optab
enum opcode_kind
{
    AXXX,

    AAAA,
    AAAD,
    AAAM,
    AAAS,
    AADCB,
    AADCL,
    AADCW,
    AADDB,
    AADDL,
    AADDW,
    AADJSP,
    AANDB,
    AANDL,
    AANDW,
    AARPL,
    ABOUNDL,
    ABOUNDW,
    ABSFL,
    ABSFW,
    ABSRL,
    ABSRW,
    ABTL,
    ABTW,
    ABTCL,
    ABTCW,
    ABTRL,
    ABTRW,
    ABTSL,
    ABTSW,

    ABYTE, //

    ACALL, //

    ACLC,
    ACLD,

    ACLI, //

    ACLTS,
    ACMC,

    ACMPB, //
    ACMPL,
    ACMPW,

    ACMPSB,
    ACMPSL,
    ACMPSW,
    ADAA,
    ADAS,

    ADATA, //


    ADECB, //
    ADECL,
    ADECW,

    ADIVB, //
    ADIVL,
    ADIVW,

    AENTER, //

    AGLOBL, //

    AGOK, //?

    AHISTORY, // for ?

    AHLT, //

    AIDIVB,
    AIDIVL,
    AIDIVW,
    AIMULB,
    AIMULL,
    AIMULW,

    AINB, //
    AINL,
    AINW,

    AINCB, //
    AINCL,
    AINCW,

    AINSB,
    AINSL,
    AINSW,
    AINT,
    AINTO,
    AIRETL,
    AIRETW,

    AJCC, //
    AJCS,
    AJCXZ,
    AJEQ,
    AJGE,
    AJGT,
    AJHI,
    AJLE,
    AJLS,
    AJLT,
    AJMI,

    AJMP, //

    AJNE,

    AJOC,
    AJOS,
    AJPC,
    AJPL,
    AJPS,
    ALAHF,
    ALARL,
    ALARW,
    ALEAL,
    ALEAW,
    ALEAVEL,
    ALEAVEW,

    ALOCK, //

    ALODSB, //
    ALODSL,
    ALODSW,

    ALONG, //

    ALOOP,
    ALOOPEQ,
    ALOOPNE,
    ALSLL,
    ALSLW,

    AMOVB, //
    AMOVL,
    AMOVW,

    AMOVBLSX,
    AMOVBLZX,
    AMOVBWSX,
    AMOVBWZX,
    AMOVWLSX,
    AMOVWLZX,
    AMOVSB,
    AMOVSL,
    AMOVSW,

    AMULB, //
    AMULL,
    AMULW,

    ANAME, // seems important

    ANEGB, //
    ANEGL,
    ANEGW,

    ANOP, //

    ANOTB, //
    ANOTL,
    ANOTW,

    AORB, //
    AORL,
    AORW,

    AOUTB, //
    AOUTL,
    AOUTW,

    AOUTSB,
    AOUTSL,
    AOUTSW,

    APOPAL,
    APOPAW,
    APOPFL,
    APOPFW,
    APOPL,
    APOPW,
    APUSHAL,
    APUSHAW,
    APUSHFL,
    APUSHFW,
    APUSHL,
    APUSHW,
    ARCLB,
    ARCLL,
    ARCLW,
    ARCRB,
    ARCRL,
    ARCRW,

    AREP,

    AREPN,

    ARET, //

    AROLB,
    AROLL,
    AROLW,
    ARORB,
    ARORL,
    ARORW,
    ASAHF,
    ASALB,
    ASALL,
    ASALW,
    ASARB,
    ASARL,
    ASARW,
    ASBBB,
    ASBBL,
    ASBBW,
    ASCASB,
    ASCASL,
    ASCASW,
    ASETCC,
    ASETCS,
    ASETEQ,
    ASETGE,
    ASETGT,
    ASETHI,
    ASETLE,
    ASETLS,
    ASETLT,
    ASETMI,
    ASETNE,
    ASETOC,
    ASETOS,
    ASETPC,
    ASETPL,
    ASETPS,
    ACDQ,
    ACWD,
    ASHLB,
    ASHLL,
    ASHLW,
    ASHRB,
    ASHRL,
    ASHRW,
    ASTC,
    ASTD,
    ASTI,

    ASTOSB, //
    ASTOSL,
    ASTOSW,

    ASUBB, //
    ASUBL,
    ASUBW,

    ASYSCALL, // !!!!!!

    ATESTB,
    ATESTL,
    ATESTW,

    ATEXT, // procedure/function TODO rename to AFUNC?

    AVERR,
    AVERW,
    AWAIT,
    AWORD,

    AXCHGB, //
    AXCHGL,
    AXCHGW,

    AXLAT,
    AXORB,
    AXORL,
    AXORW,

    AFMOVB,
    AFMOVBP,
    AFMOVD,
    AFMOVDP,
    AFMOVF,
    AFMOVFP,
    AFMOVL,
    AFMOVLP,
    AFMOVV,
    AFMOVVP,
    AFMOVW,
    AFMOVWP,
    AFMOVX,
    AFMOVXP,

    AFCOMB,
    AFCOMBP,
    AFCOMD,
    AFCOMDP,
    AFCOMDPP,
    AFCOMF,
    AFCOMFP,
    AFCOML,
    AFCOMLP,
    AFCOMW,
    AFCOMWP,
    AFUCOM,
    AFUCOMP,
    AFUCOMPP,

    AFADDDP,
    AFADDW,
    AFADDL,
    AFADDF,
    AFADDD,

    AFMULDP,
    AFMULW,
    AFMULL,
    AFMULF,
    AFMULD,

    AFSUBDP,
    AFSUBW,
    AFSUBL,
    AFSUBF,
    AFSUBD,

    AFSUBRDP,
    AFSUBRW,
    AFSUBRL,
    AFSUBRF,
    AFSUBRD,

    AFDIVDP,
    AFDIVW,
    AFDIVL,
    AFDIVF,
    AFDIVD,

    AFDIVRDP,
    AFDIVRW,
    AFDIVRL,
    AFDIVRF,
    AFDIVRD,

    AFXCHD,
    AFFREE,

    AFLDCW,
    AFLDENV,
    AFRSTOR,
    AFSAVE,
    AFSTCW,
    AFSTENV,
    AFSTSW,

    AF2XM1,
    AFABS,
    AFCHS,
    AFCLEX,
    AFCOS,
    AFDECSTP,
    AFINCSTP,
    AFINIT,
    AFLD1,
    AFLDL2E,
    AFLDL2T,
    AFLDLG2,
    AFLDLN2,
    AFLDPI,
    AFLDZ,
    AFNOP,
    AFPATAN,
    AFPREM,
    AFPREM1,
    AFPTAN,
    AFRNDINT,
    AFSCALE,
    AFSIN,
    AFSINCOS,
    AFSQRT,
    AFTST,
    AFXAM,
    AFXTRACT,
    AFYL2X,
    AFYL2XP1,

    AEND, // ??

    ADYNT,
    AINIT,

    ASIGNAME,

    AFCOMI,
    AFCOMIP,
    AFUCOMI,
    AFUCOMIP,
    ACMPXCHGB,
    ACMPXCHGL,
    ACMPXCHGW,

    /* conditional move */
    ACMOVLCC,
    ACMOVLCS,
    ACMOVLEQ,
    ACMOVLGE,
    ACMOVLGT,
    ACMOVLHI,
    ACMOVLLE,
    ACMOVLLS,
    ACMOVLLT,
    ACMOVLMI,
    ACMOVLNE,
    ACMOVLOC,
    ACMOVLOS,
    ACMOVLPC,
    ACMOVLPL,
    ACMOVLPS,
    ACMOVWCC,
    ACMOVWCS,
    ACMOVWEQ,
    ACMOVWGE,
    ACMOVWGT,
    ACMOVWHI,
    ACMOVWLE,
    ACMOVWLS,
    ACMOVWLT,
    ACMOVWMI,
    ACMOVWNE,
    ACMOVWOC,
    ACMOVWOS,
    ACMOVWPC,
    ACMOVWPL,
    ACMOVWPS,

    AFCMOVCC,
    AFCMOVCS,
    AFCMOVEQ,
    AFCMOVHI,
    AFCMOVLS,
    AFCMOVNE,
    AFCMOVNU,
    AFCMOVUN,

    /* add new operations here. nowhere else. here. */
    ALAST
};
/*e: enum as */

/*s: enum reg */
enum operand_kind
{
/*s: [[operand_kind]] register cases */
    D_AL		= 0,
    D_CL,
    D_DL,
    D_BL,

    D_AH		= 4,
    D_CH,
    D_DH,
    D_BH,

    D_AX		= 8,
    D_CX,
    D_DX,
    D_BX,
    D_SP,
    D_BP,
    D_SI,
    D_DI,

    D_F0		= 16,
    D_F7		= D_F0 + 7,

    D_CS		= 24,
    D_SS,
    D_DS,
    D_ES,
    D_FS,
    D_GS,

    D_GDTR,		/* global descriptor table register */
    D_IDTR,		/* interrupt descriptor table register */
    D_LDTR,		/* local descriptor table register */
    D_MSW,		/* machine status word */
    D_TASK,		/* task register */

    D_CR		= 35, // D_CR0 .. D_CR7
    D_DR		= 43, // D_DR0 .. D_DR7
    D_TR		= 51, // D_TR0 .. D_TR7
/*e: [[operand_kind]] register cases */
    D_NONE		= 59,
/*s: [[operand_kind]] non register cases */
    // for ACALL, AJMP (from PC)
    D_BRANCH	= 60, 

    // For ADATA
    D_EXTERN	= 61, // data/bss values (from SB)
    D_STATIC	= 62, // data static variables (from SB)
    D_AUTO	= 63, // stack values (from SP)
    D_PARAM	= 64, // parameter (from FP)

    D_CONST	= 65,
    D_FCONST	= 66,
    D_SCONST	= 67,

    D_ADDR	= 68,
/*x: [[operand_kind]] non register cases */
    D_FILE,
    D_FILE1, // used by linker only

    D_INDIR,	/* additive */

    D_CONST2 = D_INDIR+D_INDIR,

    D_SIZE,	/* 8l internal */
/*e: [[operand_kind]] non register cases */
};
/*e: enum reg */

/*s: enum misc2 */
enum misc2 {
    T_TYPE		= 1<<0,
    T_INDEX		= 1<<1,
    T_OFFSET	= 1<<2,
    T_FCONST	= 1<<3,
    T_SYM		= 1<<4,
    T_SCONST	= 1<<5,
    T_OFFSET2	= 1<<6,
};
/*e: enum misc2 */
/*s: enum misc3 */
enum misc3 {
    REGARG		= -1,
    REGRET		= D_AX,
    FREGRET		= D_F0,
    REGSP		= D_SP,
    REGTMP		= D_DI,
};
/*e: enum misc3 */

/*s: constant SYMDEF */
/*
 * this is the ranlib header
 */
#define	SYMDEF	"__.SYMDEF"
/*e: constant SYMDEF */
/*e: include/386/8.out.h */
