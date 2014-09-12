/*s: assemblers/8a/a.h */
#include "../aa/aa.h"
#include "386/8.out.h"

typedef	struct	Gen	Gen;
typedef	struct	Gen2 	Gen2;

/*s: constant FPCHIP */
#define	FPCHIP		1
/*e: constant FPCHIP */

/*s: struct Gen */
struct	Gen
{
    //??
    Sym*	sym;

    char	sval[8];
    long	offset;
    long	offset2;

    // enum?
    short	type;
    short	index;

    short	scale;

    // float
    double	dval;
};
/*e: struct Gen */
/*s: struct Gen2 */
struct	Gen2
{
    Gen	from;
    Gen	to;
};
/*e: struct Gen2 */

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
int	myfork(void);
void*	mysbrk(ulong);
/*e: assemblers/8a/a.h */
