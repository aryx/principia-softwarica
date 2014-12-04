/*s: assemblers/aa/aa.h */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ctype.h>

// aa.h  is the generic part, the specific #include XXX/Y.out.h is done
// in Ya/a.h, e.g.:
//#include "386/8.out.h"
//#include "arm/5.out.h"
#include <common.out.h>

#pragma	lib	"../aa/aa.a$O"
//$

// was originally in a XXX/Y.out.h (and was always the same in all archi)
// most of the content below was originally copy pasted in 8a/a.h, 5a/a.h, etc

typedef	struct	Sym	Sym;
typedef struct  Io Io;
typedef	struct	Hist Hist;

/*s: constant MAXALIGN */
#define MAXALIGN 7
/*e: constant MAXALIGN */
/*s: constant NSYMB */
#define	NSYMB		500
/*e: constant NSYMB */
/*s: constant BUFSIZ */
#define	BUFSIZ		8192
/*e: constant BUFSIZ */
/*s: constant HISTSZ */
#define	HISTSZ		20
/*e: constant HISTSZ */
/*s: constant NINCLUDE */
#define	NINCLUDE	10
/*e: constant NINCLUDE */
/*s: constant NHUNK */
#define	NHUNK		10000
/*e: constant NHUNK */
/*s: constant EOF */
#define	EOF		(-1)
/*e: constant EOF */
/*s: constant IGN */
#define	IGN		(-2)
/*e: constant IGN */
/*s: function GETC */
#define	GETC()		((--fi.c < 0) ? filbuf() : *fi.p++ & 0xff)
/*e: function GETC */
/*s: constant NHASH */
#define	NHASH		503
/*e: constant NHASH */
/*s: constant STRINGSZ */
#define	STRINGSZ	200
/*e: constant STRINGSZ */


/*s: struct Sym */
struct	Sym
{
    // for user labels (bar:), variables (VAR)
    // but also used for opcodes (AMOVB) and register (D_AL) keywords 
    // (see itab)
    char	*name;

    //enum<token_kind> (e.g. LNAME, LLAB, LBREG, etc)
    ushort	type;

    //enum<operand_kind> | enum<opcode> | long (e.g. pc for LLAB)
    long	value; //vlong in va/

    /*s: [[Sym]] other fields */
    //option<string>, for '#define FOO xxx' expansion
    char*	macro;
    /*x: [[Sym]] other fields */
    //?? seems used only by outcode
    char	symidx;
    /*e: [[Sym]] other fields */
    // Extra
    /*s: [[Sym]] extra fields */
    // list<ref<Sym>> (next = Sym.link) bucket of hashtbl 'hash'
    Sym*	link;
    /*e: [[Sym]] extra fields */
};
/*e: struct Sym */
/*s: constant S */
#define	S	((Sym*)nil)
/*e: constant S */

/*s: struct Fi */
struct Fi
{
    // ref<char>, pointer in buffer (of Io.b)
    char*	p;
    // remaining count in buffer (of Io.b)
    int	c;
};
/*e: struct Fi */
extern struct Fi fi;

/*s: struct Io */
struct	Io
{
    char	b[BUFSIZ];

    // -1 if not opened yet
    fdt	f;

    // like Fi, saved IO buffer status
    char*	p;
    short	c;

    // Extra
    Io*	link;
};
/*e: struct Io */
/*s: constant I */
#define	I	((Io*)nil)
/*e: constant I */

/*s: struct Htab */
struct Htab
{
    // ref<Sym>
    Sym*	sym;
    //enum<operand_kind>?
    short	type;
};
/*e: struct Htab */
extern struct Htab h[NSYM];

// Gen

/*s: struct Hist */
struct	Hist
{
    char*	name;

    long	line;
    long	offset;

    // Extra
    Hist*	link;
};
/*e: struct Hist */
/*s: constant H */
#define	H	((Hist*)0)
/*e: constant H */



// was in a.h
extern	Sym*	hash[NHASH];
extern	Hist*	hist;
extern	char*	hunk;
extern	char*	include[NINCLUDE];
extern	Io*	iofree;
extern	Io*	ionext;
extern	Io*	iostack;
extern	long	lineno;
extern	int	nerrors;
extern	long	nhunk;
extern	int	ninclude;
extern	char*	outfile;
extern	long	pc;
extern	int	peekc;
extern	int	symcounter;
extern	char	symb[NSYMB];
extern	int	thechar;

// for macbody, was in a.h
extern	bool	debug[256];
extern	Hist*	ehist;
extern	int	newflag;
extern	long	thunk;

// for lexbody, was in a.h
int	getnsc(void);
void	gethunk(void);
void	yyerror(char*, ...);
void	linehist(char*, int);
Sym*	lookup(void);
void	syminit(Sym*);
int	filbuf(void);
void	domacro(void);
void	macexpand(Sym*, char*);

// for macbody, was in a.h
int	getc(void);
void	unget(int);
Sym*	slookup(char*);

// for macbody, was in lexbody
void pragpack(void);
void pragfpround(void);
void pragprofile(void);
void pragvararg(void);
void* alloc(long n);
void pragincomplete(void);
void* allocn(void *p, long on, long n);
void	pushio(void);
void	newio(void);
void	newfile(char*, int);
void	errorexit(void);

// from lexbody.c
void ieeedtod(Ieee *ieee, double native);

/*s: enum _anon_ (assemblers/aa/aa.h) */
/*
 *	system-dependent stuff from ../cc/compat.c
 */
enum	/* keep in synch with ../cc/cc.h */
{
    Plan9	= 1<<0,
    Unix	= 1<<1,
};
/*e: enum _anon_ (assemblers/aa/aa.h) */
/*e: assemblers/aa/aa.h */
