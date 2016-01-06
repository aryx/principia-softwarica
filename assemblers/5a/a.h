/*s: assemblers/5a/a.h */
#include "../aa/aa.h"
#include <5.out.h>

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
    // enum<Operand_kind>
    short	type;

    // switch on Gen.type
    union {
        long	offset; // offset or lval or ...
        double	dval;
        char	sval[NSNAME];
    };
    // option<enum<Register>> (None = R_NONE)
    short	reg; // abused also to store a size for DATA

    /*s: [[Gen]] other fields */
    // option<ref<Sym>> (owner = hash)
    Sym*	sym;
    /*x: [[Gen]] other fields */
    // option<enum<Sym_kind>> (None = N_NONE)
    short	symkind;
    /*e: [[Gen]] other fields */
};
/*e: struct Gen(arm) */

extern	Gen	nullgen;
extern	int	pass;
extern	char*	pathname;
extern	char*	thestring;
extern	Biobuf	obuf;

extern	char*	Dlist[30];
extern	int	nDlist;


// for a.y
/*s: signature yylex */
// unit -> (enum<token_code> | -1 (EOF) | Rune)
long	yylex(void);
/*e: signature yylex */

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

// obj.c
/*s: signature outcode(arm) */
void	outcode(int opcode, int cond, Gen* opd1, int reg, Gen* opd3);
/*e: signature outcode(arm) */
void	outhist(void);
void	zname(char*, int, int);
void	zaddr(Gen*, int);

// main
void	prfile(long);
void	linehist(char*, int);
void	gethunk(void);

/*e: assemblers/5a/a.h */
