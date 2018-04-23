/*s: assemblers/8a/a.h */
#include "../aa/aa.h"
#include <8.out.h>

typedef	struct	Gen  Gen;
typedef	struct	Gen2 Gen2;

/*s: constant [[FPCHIP]](x86) */
#define	FPCHIP		true
/*e: constant [[FPCHIP]](x86) */

/*s: struct [[Gen]](x86) */
struct	Gen
{
    // enum<operand_kind>
    short	type;

    long	offset; // generic value
    double	dval;
    char	sval[8];

    //enum<operand_kind(register-only|D_NONE)>
    short	index;

    short	scale;

    //??
    Sym*	sym;

    long	offset2;
};
/*e: struct [[Gen]](x86) */
/*s: struct [[Gen2]](x86) */
struct	Gen2
{
    Gen	from;
    Gen	to;
};
/*e: struct [[Gen2]](x86) */

extern	char*	Dlist[30];
extern	int	nDlist;
extern	Gen	nullgen;
extern	int	pass;
extern	char*	pathname;
extern	char*	thestring;
extern	Biobuf	obuf;

// for a.y
long	yylex(void);
void	checkscale(int);
void	outcode(int, Gen2*);

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

// for macbody, was in lexbody

int	mywait(int*);
int	mycreat(char*, int);
int	systemtype(int);
int	pathchar(void);
/*e: assemblers/8a/a.h */
