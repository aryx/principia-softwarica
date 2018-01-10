/*s: assemblers/5a/a.h */
#include "../aa/aa.h"
#include <5.out.h>

//----------------------------------------------------------------------------
// Data structures and constants
//----------------------------------------------------------------------------

/*s: constant [[FPCHIP]](arm) */
#define FPCHIP      true
/*e: constant [[FPCHIP]](arm) */

/*s: constant [[Always]](arm) */
#define    Always 14
/*e: constant [[Always]](arm) */

/*s: struct [[Gen]](arm) */
struct  Gen
{
    // enum<Operand_kind>
    short   type;

    // switch on Gen.type
    union {
        long    offset; // offset or lval or ...
        double  dval;
        char    sval[NSNAME];
    };
    // option<enum<Register>> (None = R_NONE)
    short   reg; // abused also to store a size for DATA

    /*s: [[Gen]] other fields */
    // option<ref<Sym>> (owner = hash)
    Sym*    sym;
    /*x: [[Gen]] other fields */
    // option<enum<Sym_kind>> (None = N_NONE)
    short   symkind;
    /*e: [[Gen]] other fields */
};
/*e: struct [[Gen]](arm) */
typedef struct  Gen Gen;

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------

// globals.c
extern  Gen nullgen;

//----------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------

// lex.c (for y.tab.c, main.c)
/*s: signature [[yylex]] */
// unit -> (enum<token_code> | -1 (EOF) | char)
long    yylex(void);
/*e: signature [[yylex]] */
void    cinit(void);

// y.tab.c from a.y (for main.c)
int yyparse(void);

// obj.c (for main.c)
/*s: signature [[outcode]](arm) */
void    outcode(int opcode, int cond, Gen* opd1, int reg, Gen* opd3);
/*e: signature [[outcode]](arm) */
void    outhist(void);

/*e: assemblers/5a/a.h */
