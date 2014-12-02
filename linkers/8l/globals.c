/*s: linkers/8l/globals.c */
#include	"l.h"

/*s: global buf(x86) */
union Buf buf;
/*e: global buf(x86) */

/*s: global HEADR(x86) */
long	HEADR;
/*e: global HEADR(x86) */
/*s: global HEADTYPE(x86) */
long	HEADTYPE = -1;
/*e: global HEADTYPE(x86) */
/*s: global INITDAT(x86) */
long	INITDAT = -1;
/*e: global INITDAT(x86) */
/*s: global INITRND(x86) */
long	INITRND = -1;
/*e: global INITRND(x86) */
/*s: global INITTEXT(x86) */
long	INITTEXT = -1;
/*e: global INITTEXT(x86) */
/*s: global INITTEXTP(x86) */
long	INITTEXTP = -1;
/*e: global INITTEXTP(x86) */
/*s: global INITENTRY(x86) */
char*	INITENTRY = nil;		/* entry point */
/*e: global INITENTRY(x86) */
/*s: global bso(x86) */
Biobuf	bso;
/*e: global bso(x86) */
/*s: global bsssize(x86) */
long	bsssize;
/*e: global bsssize(x86) */
/*s: global cbc(x86) */
int	cbc;
/*e: global cbc(x86) */
/*s: global cbp(x86) */
char*	cbp;
/*e: global cbp(x86) */
/*s: global pcstr(x86) */
char*	pcstr = "%.6lux ";
/*e: global pcstr(x86) */
/*s: global cout(x86) */
fdt	cout = -1;
/*e: global cout(x86) */
/*s: global curp(x86) */
Prog*	curp;
/*e: global curp(x86) */
/*s: global curtext(x86) */
Prog*	curtext;
/*e: global curtext(x86) */
/*s: global datap(x86) */
// list<ref<Prog>>, next = Prog.next
Prog*	datap = P;
/*e: global datap(x86) */
/*s: global edatap(x86) */
// ref<Prog>>, end of datap list
Prog*	edatap = P;
/*e: global edatap(x86) */
/*s: global datsize(x86) */
long	datsize;
/*e: global datsize(x86) */
/*s: global debug(x86) */
bool	debug[128];
/*e: global debug(x86) */
/*s: global firstp(x86) */
// list<ref_own?<Prog>>, next = Prog.link
Prog*	firstp;
/*e: global firstp(x86) */
/*s: global fnuxi8(x86) */
char	fnuxi8[8];
/*e: global fnuxi8(x86) */
/*s: global fnuxi4(x86) */
char	fnuxi4[4];
/*e: global fnuxi4(x86) */
/*s: global hash (linkers/8l/globals.c) */
// hash<Sym.name * Sym.version, ref<Sym>> (next = Sym.link)
Sym*	hash[NHASH];
/*e: global hash (linkers/8l/globals.c) */
/*s: global hunk(x86) */
char*	hunk;
/*e: global hunk(x86) */
/*s: global inuxi1(x86) */
char	inuxi1[1];
/*e: global inuxi1(x86) */
/*s: global inuxi2(x86) */
char	inuxi2[2];
/*e: global inuxi2(x86) */
/*s: global inuxi4(x86) */
char	inuxi4[4];
/*e: global inuxi4(x86) */
/*s: global ycover(x86) */
char	ycover[Ymax*Ymax];
/*e: global ycover(x86) */
/*s: global andptr(x86) */
uchar*	andptr;
/*e: global andptr(x86) */
/*s: global and(x86) */
uchar	and[30];
/*e: global and(x86) */
/*s: global reg(x86) */
char	reg[D_NONE];
/*e: global reg(x86) */
/*s: global lastp(x86) */
// ref<Prog>, last elt of firstp list
Prog*	lastp;
/*e: global lastp(x86) */
/*s: global lcsize(x86) */
long	lcsize;
/*e: global lcsize(x86) */
/*s: global nerrors(x86) */
int	nerrors = 0;
/*e: global nerrors(x86) */
/*s: global nhunk(x86) */
long	nhunk;
/*e: global nhunk(x86) */
/*s: global nsymbol(x86) */
long	nsymbol;
/*e: global nsymbol(x86) */
/*s: global outfile(x86) */
char*	outfile = "8.out";
/*e: global outfile(x86) */
/*s: global pc(x86) */
long	pc = 0;
/*e: global pc(x86) */
/*s: global symsize(x86) */
long	symsize;
/*e: global symsize(x86) */
/*s: global textp(x86) */
// list<ref<Prog>>, next = Prog.cond
Prog*	textp = P;
/*e: global textp(x86) */
/*s: global textsize(x86) */
long	textsize;
/*e: global textsize(x86) */
/*s: global thunk(x86) */
long	thunk;
/*e: global thunk(x86) */
/*s: global zprg(x86) */
Prog	zprg;
/*e: global zprg(x86) */
/*s: global dtype(x86) */
int	dtype;
/*e: global dtype(x86) */

/*s: global reloca(x86) */
Adr*	reloca;
/*e: global reloca(x86) */
/*s: global dlm(x86) */
bool dlm;
/*e: global dlm(x86) */
/*s: global nimports(x86) */
int nimports;
/*e: global nimports(x86) */
/*s: global nexports(x86) */
int nexports;
/*e: global nexports(x86) */

int	imports;
int	exports;
int	allexport;
/*s: global EXPTAB(x86) */
char*	EXPTAB;
/*e: global EXPTAB(x86) */
/*s: global undefp(x86) */
//@Scheck: not dead, used by UP
Prog	undefp;
/*e: global undefp(x86) */

/*e: linkers/8l/globals.c */
