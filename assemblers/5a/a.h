/*s: assemblers/5a/a.h */
#include "../aa/aa.h"
#include <arm/5.out.h>

typedef	struct	Gen	Gen;

/*s: constant FPCHIP(arm) */
#define	FPCHIP		true
/*e: constant FPCHIP(arm) */

/*s: constant Always(arm) */
#define    Always 14
/*e: constant Always(arm) */

/*s: struct Gen(arm) */
struct	Gen
{
    // enum<operand_kind>
    short	type;

    // switch on Gen.type
    union {
        long	offset; // offset or generic value
        double	dval;
        char	sval[8];
    };
    // option<enum<registr>>, None = R_NONE
    short	reg;

    /*s: [[Gen]] other fields */
    // option<ref<Sym>>, ref owned by hash
    Sym*	sym;
    /*x: [[Gen]] other fields */
    // option<enum<sym_kind>>, None = N_NONE
    short	symkind;
    /*e: [[Gen]] other fields */
};
/*e: struct Gen(arm) */

extern	char*	Dlist[30];
extern	int	nDlist;
extern	Gen	nullgen;
extern	int	pass;
extern	char*	pathname;
extern	char*	thestring;
extern	Biobuf	obuf;

// for a.y
/*s: signature yylex */
// unit -> (enum<token_code> | -1 (EOF) | Rune)
long	yylex(void);
/*e: signature yylex */
/*s: signature outcode(arm) */
void	outcode(int opcode, int cond, Gen* opd1, int reg, Gen* opd3);
/*e: signature outcode(arm) */

int	escchar(int);
//Sym*	getsym(void);

// for lexbody
void	setinclude(char*);
void*	allocn(void*, long, long);
void	errorexit(void);
Sym*	slookup(char*);
void	pinit(char*);
void	ieeedtod(Ieee*, double);
void	dodefine(char*);
void	yyerror(char*, ...);
int	yyparse(void);

// for macbody
int	getc(void);
void	unget(int);
void	pushio(void);
void	newio(void);
void	newfile(char*, int);

int	mywait(int*);
int	mycreat(char*, int);
int	systemtype(int);
int	pathchar(void);


// TODO remove?
void*	alloc(long);
Sym*	lookup(void);
void	syminit(Sym*);
int	getnsc(void);
void	cinit(void);
void	cclean(void);
void	zname(char*, int, int);
void	zaddr(Gen*, int);
int	filbuf(void);

void	domacro(void);
void	macund(void);
void	macdef(void);
void	macexpand(Sym*, char*);
void	macinc(void);
void	maclin(void);
void	macprag(void);
void	macif(int);
void	macend(void);

void	outhist(void);
void	prfile(long);
void	linehist(char*, int);
void	gethunk(void);
int	assemble(char*);

/*e: assemblers/5a/a.h */
