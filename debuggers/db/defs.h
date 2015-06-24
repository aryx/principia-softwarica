/*s: db/defs.h */
/*
 * adb - common definitions
 * something of a grab-bag
 */

#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ctype.h>

#include <mach.h>

/*s: type ADDR */
typedef uvlong ADDR;
/*e: type ADDR */
/*s: type WORD */
typedef ulong WORD;
/*e: type WORD */

typedef struct bkpt	BKPT;

/*s: constant HUGEINT */
#define	HUGEINT	0x7fffffff	/* enormous WORD */
/*e: constant HUGEINT */

/*s: constant MAXOFF */
#define	MAXOFF	0x1000000
/*e: constant MAXOFF */
/*s: constant INCDIR */
#define	INCDIR	"/usr/lib/adb"
/*e: constant INCDIR */
/*s: constant DBNAME */
#define	DBNAME	"db\n"
/*e: constant DBNAME */
/*s: constant CMD_VERBS */
#define CMD_VERBS	"?/=>!$: \t"
/*e: constant CMD_VERBS */

/*s: constant MAXPOS */
#define MAXPOS	80
/*e: constant MAXPOS */
/*s: constant MAXLIN */
#define MAXLIN	128
/*e: constant MAXLIN */
/*s: constant ARB */
#define	ARB	512
/*e: constant ARB */
/*s: constant MAXCOM */
#define MAXCOM	64
/*e: constant MAXCOM */
/*s: constant MAXARG */
#define MAXARG	32
/*e: constant MAXARG */
/*s: constant LINSIZ */
#define LINSIZ	4096
/*e: constant LINSIZ */
/*s: constant MAXSYM */
#define	MAXSYM	255
/*e: constant MAXSYM */

/*s: constant EOR */
#define EOR	'\n'
/*e: constant EOR */
/*s: constant SPC */
#define SPC	' '
/*e: constant SPC */
/*s: constant TB */
#define TB	'\t'
/*e: constant TB */

/*s: constant TRUE */
#define	TRUE	(-1)
/*e: constant TRUE */
/*s: constant FALSE */
#define	FALSE	0
/*e: constant FALSE */


/*
 * run modes
 */
/*s: constant SINGLE */
#define	SINGLE	1
/*e: constant SINGLE */
/*s: constant CONTIN */
#define	CONTIN	2
/*e: constant CONTIN */

/*
 * breakpoints
 */
/*s: constant BKPTCLR */
#define	BKPTCLR	0	/* not a real breakpoint */
/*e: constant BKPTCLR */
/*s: constant BKPTSET */
#define BKPTSET	1	/* real, ready to trap */
/*e: constant BKPTSET */
/*s: constant BKPTSKIP */
#define BKPTSKIP 2	/* real, skip over it next time */
/*e: constant BKPTSKIP */
/*s: constant BKPTTMP */
#define	BKPTTMP	3	/* temporary; clear when it happens */
/*e: constant BKPTTMP */

/*s: struct bkpt */
struct bkpt {
    // address/symbol to break on
    ADDR	loc;
    // enum<breakpoint_kind>
    int	flag;

    /*s: [[Bkpt]] other fields */
    uchar	save[4];

    int	count;
    int	initcnt;
    /*x: [[Bkpt]] other fields */
    char	comm[MAXCOM];
    /*e: [[Bkpt]] other fields */

    // Extra
    /*s: [[Bkpt]] extra fields */
    // list<ref_own<BKPT>, head = bkpthead
    BKPT	*nxtbkpt;
    /*e: [[Bkpt]] extra fields */
};
/*e: struct bkpt */

/*s: constant BADREG */
#define	BADREG	(-1)
/*e: constant BADREG */

/*
 * common globals
 */

extern	WORD	adrval;
extern	uvlong	expv;
extern	int	adrflg;
extern	WORD	cntval;
extern	int	cntflg;
extern	WORD	loopcnt;
extern	ADDR	maxoff;
extern	ADDR	localval;
extern	ADDR	maxfile;
extern	ADDR	maxstor;

extern	ADDR	dot;
extern	int	dotinc;

extern	int	xargc;

extern	bool	wtflag;
extern	char	*corfil, *symfil;
extern	int	fcor, fsym;
extern	bool	mkfault;
extern	bool	regdirty;

extern	int	pid;
extern	int	pcsactive;

/*s: constant NNOTE */
#define	NNOTE 10
/*e: constant NNOTE */
extern	int	nnote;
extern	char	note[NNOTE][ERRMAX];

extern	int	ending;
extern	Map	*cormap, *symmap, *dotmap;

extern	BKPT	*bkpthead;
extern	bool	kflag;
extern	int	lastc, peekc;

// new decl, was in main.c before
extern char *errmsg;
extern jmp_buf env;

extern ADDR	ditto;
/*e: db/defs.h */
