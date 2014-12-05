/*s: linkers/5l/globals.c */
#include	"l.h"

/*s: global buf(arm) */
union Buf buf;
/*e: global buf(arm) */


/*s: global HEADR(arm) */
long	HEADR;			/* length of header */
/*e: global HEADR(arm) */
/*s: global HEADTYPE(arm) */
int	HEADTYPE;		/* type of header */
/*e: global HEADTYPE(arm) */
/*s: global INITDAT(arm) */
long	INITDAT;		/* data location */
/*e: global INITDAT(arm) */
/*s: global INITRND(arm) */
long	INITRND;		/* data round above text location */
/*e: global INITRND(arm) */
/*s: global INITTEXT(arm) */
long	INITTEXT;		/* text location */
/*e: global INITTEXT(arm) */
/*s: global INITTEXTP(arm) */
long	INITTEXTP;		/* text location (physical) */
/*e: global INITTEXTP(arm) */
/*s: global INITENTRY(arm) */
char*	INITENTRY;		/* entry point */
/*e: global INITENTRY(arm) */
/*s: global autosize(arm) */
long	autosize;
/*e: global autosize(arm) */
/*s: global bso(arm) */
Biobuf	bso;
/*e: global bso(arm) */
/*s: global bsssize(arm) */
long	bsssize;
/*e: global bsssize(arm) */
/*s: global cbc(arm) */
int	cbc;
/*e: global cbc(arm) */
/*s: global cbp(arm) */
uchar*	cbp;
/*e: global cbp(arm) */
/*s: global cout(arm) */
int	cout;
/*e: global cout(arm) */
/*s: global curauto(arm) */
Auto*	curauto;
/*e: global curauto(arm) */
/*s: global curhist(arm) */
Auto*	curhist;
/*e: global curhist(arm) */
/*s: global curp(arm) */
Prog*	curp;
/*e: global curp(arm) */
/*s: global curtext(arm) */
Prog*	curtext;
/*e: global curtext(arm) */
/*s: global datap(arm) */
Prog*	datap;
/*e: global datap(arm) */
/*s: global datsize(arm) */
long	datsize;
/*e: global datsize(arm) */
/*s: global debug(arm) */
char	debug[128];
/*e: global debug(arm) */
/*s: global etextp(arm) */
Prog*	etextp;
/*e: global etextp(arm) */
/*s: global firstp(arm) */
Prog*	firstp;
/*e: global firstp(arm) */
/*s: global fnuxi4(arm) */
char	fnuxi4[4];
/*e: global fnuxi4(arm) */
/*s: global fnuxi8(arm) */
char	fnuxi8[8];
/*e: global fnuxi8(arm) */
/*s: global noname(arm) */
char*	noname;
/*e: global noname(arm) */
/*s: global hash(arm) */
Sym*	hash[NHASH];
/*e: global hash(arm) */
/*s: global histfrog(arm) */
Sym*	histfrog[MAXHIST];
/*e: global histfrog(arm) */
/*s: global histfrogp(arm) */
int	histfrogp;
/*e: global histfrogp(arm) */
/*s: global histgen(arm) */
int	histgen;
/*e: global histgen(arm) */
/*s: global library(arm) */
char*	library[50];
/*e: global library(arm) */
/*s: global libraryobj(arm) */
char*	libraryobj[50];
/*e: global libraryobj(arm) */
/*s: global libraryp(arm) */
int	libraryp;
/*e: global libraryp(arm) */
/*s: global xrefresolv(arm) */
int	xrefresolv;
/*e: global xrefresolv(arm) */
/*s: global hunk(arm) */
char*	hunk;
/*e: global hunk(arm) */
/*s: global inuxi1(arm) */
char	inuxi1[1];
/*e: global inuxi1(arm) */
/*s: global inuxi2(arm) */
char	inuxi2[2];
/*e: global inuxi2(arm) */
/*s: global inuxi4(arm) */
char	inuxi4[4];
/*e: global inuxi4(arm) */
/*s: global lastp(arm) */
Prog*	lastp;
/*e: global lastp(arm) */
/*s: global lcsize(arm) */
long	lcsize;
/*e: global lcsize(arm) */
/*s: global literal(arm) */
char	literal[32];
/*e: global literal(arm) */
/*s: global nerrors(arm) */
int	nerrors;
/*e: global nerrors(arm) */
/*s: global nhunk(arm) */
long	nhunk;
/*e: global nhunk(arm) */
/*s: global instoffset(arm) */
long	instoffset;
/*e: global instoffset(arm) */
/*s: global opcross(arm) */
Opcross	opcross[8];
/*e: global opcross(arm) */
/*s: global oprange(arm) */
Oprang	oprange[ALAST];
/*e: global oprange(arm) */
/*s: global outfile(arm) */
char*	outfile;
/*e: global outfile(arm) */
/*s: global pc(arm) */
long	pc;
/*e: global pc(arm) */
/*s: global repop(arm) */
uchar	repop[ALAST];
/*e: global repop(arm) */
/*s: global symsize(arm) */
long	symsize;
/*e: global symsize(arm) */
/*s: global textp(arm) */
Prog*	textp;
/*e: global textp(arm) */
/*s: global textsize(arm) */
long	textsize;
/*e: global textsize(arm) */
/*s: global thunk(arm) */
long	thunk;
/*e: global thunk(arm) */
/*s: global version(arm) */
int	version;
/*e: global version(arm) */
/*s: global xcmp(arm) */
char	xcmp[C_GOK+1][C_GOK+1];
/*e: global xcmp(arm) */
/*s: global zprg(arm) */
Prog	zprg;
/*e: global zprg(arm) */
/*s: global dtype(arm) */
int	dtype;
/*e: global dtype(arm) */
/*s: global armv4(arm) */
int	armv4;
/*e: global armv4(arm) */
/*s: global vfp(arm) */
int vfp;
/*e: global vfp(arm) */

/*s: global doexp(arm) */
int	doexp;
/*e: global doexp(arm) */
/*s: global dlm(arm) */
int dlm;
/*e: global dlm(arm) */
/*s: global imports(arm) */
int	imports;
/*e: global imports(arm) */
/*s: global nimports(arm) */
int nimports;
/*e: global nimports(arm) */
/*s: global exports(arm) */
int	exports;
/*e: global exports(arm) */
/*s: global nexports(arm) */
int nexports;
/*e: global nexports(arm) */
/*s: global EXPTAB(arm) */
char*	EXPTAB;
/*e: global EXPTAB(arm) */
/*s: global undefp(arm) */
Prog	undefp;
/*e: global undefp(arm) */

/*s: global anames (linkers/5l/globals.c)(arm) */
char*	anames[];
/*e: global anames (linkers/5l/globals.c)(arm) */
/*s: global optab(arm) */
Optab	optab[];
/*e: global optab(arm) */

/*s: global blitrl(arm) */
Prog*	blitrl;
/*e: global blitrl(arm) */
/*s: global elitrl(arm) */
Prog*	elitrl;
/*e: global elitrl(arm) */

/*s: global prog_div(arm) */
Prog*	prog_div;
/*e: global prog_div(arm) */
/*s: global prog_divu(arm) */
Prog*	prog_divu;
/*e: global prog_divu(arm) */
/*s: global prog_mod(arm) */
Prog*	prog_mod;
/*e: global prog_mod(arm) */
/*s: global prog_modu(arm) */
Prog*	prog_modu;
/*e: global prog_modu(arm) */
/*e: linkers/5l/globals.c */
