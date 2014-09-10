/*s: generators/lex/ldefs.h */
#include <u.h>
#include <libc.h>
#include <ctype.h>
#include <bio.h>

/*s: constant PP */
#define PP 1
/*e: constant PP */

/*s: constant PC */
#define PC 1
/*e: constant PC */
/*s: constant PS */
#define PS 1
/*e: constant PS */

/*s: constant CWIDTH */
#define CWIDTH 8
/*e: constant CWIDTH */
/*s: constant CMASK */
#define CMASK 0377
/*e: constant CMASK */
/*s: constant NCH */
#define NCH 256
/*e: constant NCH */


/*s: constant TOKENSIZE */
#define TOKENSIZE 1000
/*e: constant TOKENSIZE */
/*s: constant DEFSIZE */
#define DEFSIZE 40
/*e: constant DEFSIZE */
/*s: constant DEFCHAR */
#define DEFCHAR 1000
/*e: constant DEFCHAR */
/*s: constant STARTCHAR */
#define STARTCHAR 100
/*e: constant STARTCHAR */
/*s: constant STARTSIZE */
#define STARTSIZE 256
/*e: constant STARTSIZE */
/*s: constant CCLSIZE */
#define CCLSIZE 1000
/*e: constant CCLSIZE */

/*s: constant TREESIZE */
#define TREESIZE 1000
/*e: constant TREESIZE */
/*s: constant NSTATES */
#define NSTATES 500
/*e: constant NSTATES */
/*s: constant MAXPOS */
#define MAXPOS 2500
/*e: constant MAXPOS */
/*s: constant NTRANS */
#define NTRANS 2000
/*e: constant NTRANS */
/*s: constant NOUTPUT */
#define NOUTPUT 5000
/*e: constant NOUTPUT */

/*s: constant NACTIONS */
#define NACTIONS 100
/*e: constant NACTIONS */
/*s: constant ALITTLEEXTRA */
#define ALITTLEEXTRA 30
/*e: constant ALITTLEEXTRA */

/*s: constant RCCL */
#define RCCL NCH+90
/*e: constant RCCL */
/*s: constant RNCCL */
#define RNCCL NCH+91
/*e: constant RNCCL */
/*s: constant RSTR */
#define RSTR NCH+92
/*e: constant RSTR */
/*s: constant RSCON */
#define RSCON NCH+93
/*e: constant RSCON */
/*s: constant RNEWE */
#define RNEWE NCH+94
/*e: constant RNEWE */
/*s: constant FINAL */
#define FINAL NCH+95
/*e: constant FINAL */
/*s: constant RNULLS */
#define RNULLS NCH+96
/*e: constant RNULLS */
/*s: constant RCAT */
#define RCAT NCH+97
/*e: constant RCAT */
/*s: constant STAR */
#define STAR NCH+98
/*e: constant STAR */
/*s: constant PLUS */
#define PLUS NCH+99
/*e: constant PLUS */
/*s: constant QUEST */
#define QUEST NCH+100
/*e: constant QUEST */
/*s: constant DIV */
#define DIV NCH+101
/*e: constant DIV */
/*s: constant BAR */
#define BAR NCH+102
/*e: constant BAR */
/*s: constant CARAT */
#define CARAT NCH+103
/*e: constant CARAT */
/*s: constant S1FINAL */
#define S1FINAL NCH+104
/*e: constant S1FINAL */
/*s: constant S2FINAL */
#define S2FINAL NCH+105
/*e: constant S2FINAL */

/*s: constant DEFSECTION */
#define DEFSECTION 1
/*e: constant DEFSECTION */
/*s: constant RULESECTION */
#define RULESECTION 2
/*e: constant RULESECTION */
/*s: constant ENDSECTION */
#define ENDSECTION 5
/*e: constant ENDSECTION */

/*s: constant TRUE */
#define TRUE 1
/*e: constant TRUE */
/*s: constant FALSE */
#define FALSE 0
/*e: constant FALSE */


//#define DEBUG 1

//#ifdef DEBUG
extern int debug;		/* 1 = on */
extern int charc;
/*s: constant LINESIZE */
#define LINESIZE 110
/*e: constant LINESIZE */
//#endif

#ifdef DEBUG
extern int yydebug;
#endif

#ifdef DEBUG
extern int	freturn(int);
#else
/*s: function freturn */
#define freturn(s) s
/*e: function freturn */
#endif

extern int sargc;
extern char **sargv;
extern uchar buf[520];
extern int yyline;		/* line number of file */
extern char *yyfile;		/* file name of file */
extern int sect;
extern int eof;
extern int lgatflg;
extern int divflg;
extern int funcflag;
extern int pflag;
extern int casecount;
extern int chset;	/* 1 = char set modified */
extern Biobuf *fin, fout, *fother;
extern int foutopen;
extern int errorf;
extern int fptr;
extern char *cname;
extern int prev;	/* previous input character */
extern int pres;	/* present input character */
extern int peek;	/* next input character */
extern int *name;
extern int *left;
extern int *right;
extern int *parent;
extern uchar **ptr;
extern uchar *nullstr;
extern int tptr;
extern uchar pushc[TOKENSIZE];
extern uchar *pushptr;
extern uchar slist[STARTSIZE];
extern uchar *slptr;
extern uchar **def, **subs, *dchar;
extern uchar **sname, *stchar;
extern uchar *ccl;
extern uchar *ccptr;
extern uchar *dp, *sp;
extern int dptr, sptr;
extern uchar *bptr;		/* store input position */
extern uchar *tmpstat;
extern int count;
extern int **foll;
extern int *nxtpos;
extern int *positions;
extern int *gotof;
extern int *nexts;
extern uchar *nchar;
extern int **state;
extern int *sfall;		/* fallback state num */
extern uchar *cpackflg;		/* true if state has been character packed */
extern int *atable, aptr;
extern int nptr;
extern uchar symbol[NCH];
extern uchar cindex[NCH];
extern int xstate;
extern int stnum;
extern int ccount;
extern uchar match[NCH];
extern uchar extra[NACTIONS];
extern uchar *pcptr, *pchar;
extern int pchlen;
extern int nstates, maxpos;
extern int yytop;
extern int report;
extern int ntrans, treesize, outsize;
extern long rcount;
extern int *verify, *advance, *stoff;
extern int scon;
extern uchar *psave;

extern int nine;

extern void	acompute(int);
extern void	add(int **, int);
extern void	allprint(int);
extern void	cclinter(int);
extern void	cgoto(void);
extern void	cfoll(int);
extern int	lcpyact(void);
extern int	dupl(int);
extern void	error(char *,...);
extern void	first(int);
extern void	follow(int);
extern int	gch(void);
extern uchar	*getl(uchar *);
extern void	layout(void);
extern void	lgate(void);
extern int	lookup(uchar *, uchar **);
extern int	member(int, uchar *);
extern void	mkmatch(void);
extern int	mn0(int);
extern int	mn1(int, int);
extern int	mnp(int, void*);
extern int	mn2(int, int, uintptr);
extern void	munputc(int);
extern void	munputs(uchar *);
extern void	*myalloc(int, int);
extern void	nextstate(int, int);
extern int	notin(int);
extern void	packtrans(int, uchar *, int *, int, int);
extern void	padd(int **, int);
extern void	pccl(void);
extern void	pfoll(void);
extern void	phead1(void);
extern void	phead2(void);
extern void	pstate(int);
extern void	ptail(void);
extern void	sect1dump(void);
extern void	sect2dump(void);
extern void	statistics(void);
extern void	stprt(int);
extern void	strpt(uchar *);
extern void	treedump(void);
extern int	usescape(int);
extern void	warning(char *,...);
extern int	yyparse(void);
extern void	yyerror(char *);
/*e: generators/lex/ldefs.h */
