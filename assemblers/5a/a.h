/*s: assemblers/5a/a.h */
#include "../aa/aa.h"
#include "arm/5.out.h"

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

    long	offset; // generic value
    double	dval;
    char	sval[8];
    /*s: [[Gen]] other fields */
    // option<enum<register>> where None is done via NREG
    short	reg; // ??

    // option<Sym>, for debubbing purpose?
    Sym*	sym;
    // option<name_kind> None = N_NONE
    short	name;
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
long	yylex(void);
void	outcode(int, int, Gen*, int, Gen*);

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
