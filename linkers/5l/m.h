/*s: linkers/5l/m.h */

typedef struct  Optab   Optab;
typedef struct  Oprange Oprange;

/*s: enum Operand_class(arm) */
// order of entries for one kind matters! coupling with cmp() and ocmp()
enum Operand_class {
    C_NONE      = 0,

    C_REG,     // D_REG
    C_BRANCH,  // D_BRANCH

    // D_CONST
    /*s: [[Operand_class]] C_xCON cases */
    C_RCON,     /* 0xff rotated */ // [0..0xff] range, possibly rotated
    C_NCON,     /* ~RCON */
    C_LCON,
    /*e: [[Operand_class]] C_xCON cases */

    // D_OREG
    /*s: [[Operand_class]] C_xOREG cases */
    /*s: [[Operand_class]] cases, in C_xOREG, half case */
    C_HOREG,
    /*e: [[Operand_class]] cases, in C_xOREG, half case */
    /*s: [[Operand_class]] cases, in C_xOREG, float cases */
    C_FOREG,
    C_HFOREG,
    /*e: [[Operand_class]] cases, in C_xOREG, float cases */
    C_SOREG,
    C_LOREG,

    C_ROREG,
    C_SROREG,   /* both S and R */
    /*e: [[Operand_class]] C_xOREG cases */
    // D_OREG with symbol N_EXTERN (or N_INTERN)
    /*s: [[Operand_class]] C_xEXT cases */
    /*s: [[Operand_class]] cases, in C_xEXT, half case */
    C_HEXT,
    /*e: [[Operand_class]] cases, in C_xEXT, half case */
    /*s: [[Operand_class]] cases, in C_xEXT, float cases */
    C_FEXT,
    C_HFEXT,
    /*e: [[Operand_class]] cases, in C_xEXT, float cases */
    C_SEXT,
    C_LEXT,
    /*e: [[Operand_class]] C_xEXT cases */
    // D_OREG with symbol N_PARAM or N_LOCAL
    /*s: [[Operand_class]] C_xAUTO cases */
    /*s: [[Operand_class]] cases, in C_xAUTO, half case */
    C_HAUTO,    /* halfword insn offset (-0xff to 0xff) */
    /*e: [[Operand_class]] cases, in C_xAUTO, half case */
    /*s: [[Operand_class]] cases, in C_xAUTO, float cases */
    C_FAUTO,    /* float insn offset (0 to 0x3fc, word aligned) */
    C_HFAUTO,   /* both H and F */
    /*e: [[Operand_class]] cases, in C_xAUTO, float cases */
    C_SAUTO,    /* -0xfff to 0xfff */
    C_LAUTO,
    /*e: [[Operand_class]] C_xAUTO cases */

    // D_ADDR
    /*s: [[Operand_class]] C_xxCON cases */
    C_RECON,
    /*x: [[Operand_class]] C_xxCON cases */
    C_RACON,
    C_LACON,
    /*e: [[Operand_class]] C_xxCON cases */

    /*s: [[Operand_class]] cases */
    C_SHIFT,   // D_SHIFT
    /*x: [[Operand_class]] cases */
    C_ADDR,     /* relocatable address */
    /*x: [[Operand_class]] cases */
    C_FREG,
    C_FCON,
    C_FCR,
    /*x: [[Operand_class]] cases */
    C_REGREG,  // D_REGREG
    /*x: [[Operand_class]] cases */
    C_PSR,     // D_PSR
    /*e: [[Operand_class]] cases */
    C_GOK, // must be at the end e.g., for xcmp[] decl, or buildop loops
};
/*e: enum Operand_class(arm) */

/*s: struct Optab(arm) */
struct  Optab
{
    // ---------------------------
    // The pattern (if)
    // ---------------------------

    // enum<Opcode> (the opcode is the representant of a set of opcodes)
    byte    as;

    // enum<Operand_class>, possible operand class for first operand (from)
    short   a1;
    // enum<Operand_class>, possible operand class for middle operand
    short   a2;
    // enum<Operand_class>, possible operand class for third operand (to)
    short   a3;

    // ---------------------------
    // The action (then)
    // ---------------------------

    // action id for the code generator (see the giant switch in asmout())
    short   type; 
    // size of the corresponding machine code (should be a multiple of 4)
    short   size; 

    /*s: [[Optab]] param field */
    // 0 | REGSB | REGSP
    short   param;
    /*e: [[Optab]] param field */
    /*s: [[Optab]] flag field */
    // bitset<enum<Optab_flag>>
    short   flag;
    /*e: [[Optab]] flag field */
};
/*e: struct Optab(arm) */

/*s: struct Oprange(arm) */
struct  Oprange
{
    //starting index in (sorted) optab
    Optab*  start;
    //ending index in (sorted) optab
    Optab*  stop;
};
/*e: struct Oprange(arm) */

/*s: enum Optab_flag(arm) */
enum Optab_flag {
    // Flags related to literal pools
    LFROM   = 1<<0,
    LTO     = 1<<1,

    LPOOL   = 1<<2,
    // Flags related architecture restrictions
    /*s: [[Optab_flag]] cases */
    VFP     = 1<<4, /* arm vfpv3 floating point */
    /*x: [[Optab_flag]] cases */
    V4      = 1<<3, /* arm v4 arch */
    /*e: [[Optab_flag]] cases */
};
/*e: enum Optab_flag(arm) */

// globals
extern  Optab   optab[];
extern  Oprange oprange[ALAST];
extern  long    instoffset;

extern  bool    xcmp[C_GOK+1][C_GOK+1];

// span.c
Optab*  oplook(Prog*);

/*e: linkers/5l/m.h */
