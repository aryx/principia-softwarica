/*s: linkers/5l/m.h */

typedef struct  Optab   Optab;
typedef struct  Oprange Oprange;

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

/*e: linkers/5l/m.h */
