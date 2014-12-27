/*s: linkers/5l/globals.c */
#include	"l.h"

/*s: global buf */
union Buf buf;
/*e: global buf */


/*s: global HEADR */
long	HEADR; 		/* length of header */
/*e: global HEADR */
/*s: global HEADTYPE */
// enum<headtype>
short	HEADTYPE = -1; /* type of header */
/*e: global HEADTYPE */
/*s: global INITDAT */
long	INITDAT = -1; /* data location */
/*e: global INITDAT */
/*s: global INITRND */
long	INITRND = -1; 		/* data round above text location */
/*e: global INITRND */
/*s: global INITTEXT */
long	INITTEXT = -1; /* text location */
/*e: global INITTEXT */
/*s: global INITTEXTP */
long	INITTEXTP = -1; /* text location (physical) */
/*e: global INITTEXTP */
/*s: global INITENTRY */
char*	INITENTRY = nil;		/* entry point */
/*e: global INITENTRY */

/*s: global autosize(arm) */
long	autosize;
/*e: global autosize(arm) */
/*s: global bso */
Biobuf	bso;
/*e: global bso */
/*s: global bsssize */
long	bsssize;
/*e: global bsssize */

/*s: global cbc */
int	cbc;
/*e: global cbc */
/*s: global cbp */
char*	cbp;
/*e: global cbp */

/*s: global cout */
fdt	cout = -1;
/*e: global cout */
/*s: global curauto */
Auto*	curauto;
/*e: global curauto */
/*s: global curhist */
Auto*	curhist;
/*e: global curhist */
/*s: global curp */
Prog*	curp;
/*e: global curp */
/*s: global curtext */
Prog*	curtext = P;
/*e: global curtext */
/*s: global datap */
// list<ref<Prog>>, next = Prog.next
Prog*	datap = P;
/*e: global datap */
/*s: global datsize */
long	datsize;
/*e: global datsize */
/*s: global debug */
bool	debug[128];
/*e: global debug */
/*s: global etextp */
// ref<Prog>, end of textp list
Prog*	etextp = P;
/*e: global etextp */
/*s: global firstp */
// list<ref_own?<Prog>>, next = Prog.link
Prog*	firstp;
/*e: global firstp */

/*s: global fnuxi4 */
char	fnuxi4[4];
/*e: global fnuxi4 */
/*s: global fnuxi8 */
char	fnuxi8[8];
/*e: global fnuxi8 */

/*s: global hash linker */
// hash<Sym.name * Sym.version, ref<Sym>> (next = Sym.link)
Sym*	hash[NHASH];
/*e: global hash linker */

/*s: global histfrog */
Sym*	histfrog[MAXHIST];
/*e: global histfrog */
/*s: global histfrogp */
int	histfrogp;
/*e: global histfrogp */
/*s: global histgen */
int	histgen = 0;
/*e: global histgen */

/*s: global library */
// array<option<filename>>
char*	library[50];
/*e: global library */
/*s: global libraryobj */
char*	libraryobj[50];
/*e: global libraryobj */
/*s: global libraryp */
// index of first free entry in library array
int	libraryp;
/*e: global libraryp */

/*s: global xrefresolv */
bool	xrefresolv;
/*e: global xrefresolv */
/*s: global hunk */
char*	hunk;
/*e: global hunk */

/*s: global inuxi1 */
char	inuxi1[1];
/*e: global inuxi1 */
/*s: global inuxi2 */
char	inuxi2[2];
/*e: global inuxi2 */
/*s: global inuxi4 */
char	inuxi4[4];
/*e: global inuxi4 */

/*s: global lastp */
// ref<Prog>, last elt of firstp list
Prog*	lastp;
/*e: global lastp */
/*s: global lcsize */
long	lcsize;
/*e: global lcsize */
/*s: global literal(arm) */
char	literal[32];
/*e: global literal(arm) */
/*s: global nerrors */
int	nerrors = 0;
/*e: global nerrors */
/*s: global nhunk */
long	nhunk;
/*e: global nhunk */
/*s: global nsymbol linker */
long	nsymbol;
/*e: global nsymbol linker */
/*s: global instoffset(arm) */
long	instoffset;
/*e: global instoffset(arm) */
/*s: global opcross(arm) */
Opcross	opcross[8];
/*e: global opcross(arm) */
/*s: global oprange(arm) */
Oprang	oprange[ALAST];
/*e: global oprange(arm) */
/*s: global outfile */
char*	outfile;
/*e: global outfile */
/*s: global pc */
long	pc = 0;
/*e: global pc */
/*s: global repop(arm) */
uchar	repop[ALAST];
/*e: global repop(arm) */
/*s: global symsize */
long	symsize;
/*e: global symsize */
/*s: global textp */
// list<ref<Prog>>, next = Prog.cond
Prog*	textp = P;
/*e: global textp */
/*s: global textsize */
long	textsize;
/*e: global textsize */
/*s: global thunk */
long	thunk;
/*e: global thunk */
/*s: global version */
int	version = 0;
/*e: global version */
/*s: global xcmp(arm) */
char	xcmp[C_GOK+1][C_GOK+1];
/*e: global xcmp(arm) */
/*s: global zprg */
Prog	zprg;
/*e: global zprg */
/*s: global dtype(arm) */
int	dtype = 4;
/*e: global dtype(arm) */
/*s: global armv4(arm) */
bool	armv4;
/*e: global armv4(arm) */
/*s: global vfp(arm) */
bool vfp;
/*e: global vfp(arm) */

/*s: global doexp */
// do export table, -x
bool	doexp;
/*e: global doexp */
/*s: global dlm */
bool dlm;
/*e: global dlm */

/*s: global imports */
int	imports;
/*e: global imports */
/*s: global nimports */
int nimports;
/*e: global nimports */
/*s: global exports */
int	exports;
/*e: global exports */
/*s: global nexports */
int nexports;
/*e: global nexports */
/*s: global EXPTAB */
char*	EXPTAB;
/*e: global EXPTAB */
/*s: global undefp */
//@Scheck: not dead, used by UP
Prog	undefp;
/*e: global undefp */

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
