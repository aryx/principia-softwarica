/*s: assemblers/8a/a.h */
#include "../aa/aa.h"
#include "386/8.out.h"

typedef	struct	Gen	Gen;
typedef	struct	Gen2 	Gen2;

/*s: constant FPCHIP */
#define	FPCHIP		1
/*e: constant FPCHIP */
/*s: constant NMACRO */
#define	NMACRO		10
/*e: constant NMACRO */


/*s: struct Gen */
struct	Gen
{
    double	dval;
    char	sval[8];
    long	offset;
    long	offset2;
    Sym*	sym;
    short	type;
    short	index;
    short	scale;
};
/*e: struct Gen */
/*s: struct Gen2 */
struct	Gen2
{
    Gen	from;
    Gen	to;
};
/*e: struct Gen2 */


/*s: enum _anon_ (assemblers/8a/a.h) */
enum
{
    CLAST,
    CMACARG,
    CMACRO,
    CPREPROC,
};
/*e: enum _anon_ (assemblers/8a/a.h) */


extern	char*	Dlist[30];
extern	int	nDlist;
extern	Gen	nullgen;
extern	int	pass;
extern	char*	pathname;
extern	char*	thestring;
extern	Biobuf	obuf;

long	yylex(void);
int	escchar(int);
void	cinit(void);
void	checkscale(int);
void	cclean(void);
int	isreg(Gen*);
void	outcode(int, Gen2*);
void	outhist(void);
void	zaddr(Gen*, int);
void	zname(char*, int, int);
Sym*	getsym(void);
int	assemble(char*);

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
void	macund(void);
void	macdef(void);
void	macinc(void);
void	macprag(void);
void	maclin(void);
void	macif(int);
void	macend(void);
void	pushio(void);
void	newio(void);
void	newfile(char*, int);

// for macbody, was in lexbody

int	mywait(int*);
int	mycreat(char*, int);
int	systemtype(int);
int	pathchar(void);
char*	mygetwd(char*, int);
int	myexec(char*, char*[]);
int	mydup(int, int);
int	myfork(void);
int	mypipe(int*);
void*	mysbrk(ulong);
/*e: assemblers/8a/a.h */
