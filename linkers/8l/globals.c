/*s: linkers/8l/globals.c */
#include	"l.h"

/*s: global buf */
union Buf buf;
/*e: global buf */

/*s: global HEADR */
long	HEADR;
/*e: global HEADR */
/*s: global HEADTYPE */
long	HEADTYPE = -1;
/*e: global HEADTYPE */
/*s: global INITDAT */
long	INITDAT = -1;
/*e: global INITDAT */
/*s: global INITRND */
long	INITRND = -1;
/*e: global INITRND */
/*s: global INITTEXT */
long	INITTEXT = -1;
/*e: global INITTEXT */
/*s: global INITTEXTP */
long	INITTEXTP = -1;
/*e: global INITTEXTP */
/*s: global INITENTRY */
char*	INITENTRY = nil;		/* entry point */
/*e: global INITENTRY */
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
/*s: global pcstr */
char*	pcstr = "%.6lux ";
/*e: global pcstr */
/*s: global cout */
fdt	cout = -1;
/*e: global cout */
/*s: global curp */
Prog*	curp;
/*e: global curp */
/*s: global curtext */
Prog*	curtext;
/*e: global curtext */
/*s: global datap */
// list<ref<Prog>>, next = Prog.next
Prog*	datap = P;
/*e: global datap */
/*s: global edatap */
// ref<Prog>>, end of datap list
Prog*	edatap = P;
/*e: global edatap */
/*s: global datsize */
long	datsize;
/*e: global datsize */
/*s: global debug */
bool	debug[128];
/*e: global debug */
/*s: global firstp */
// list<ref_own?<Prog>>, next = Prog.link
Prog*	firstp;
/*e: global firstp */
/*s: global fnuxi8 */
char	fnuxi8[8];
/*e: global fnuxi8 */
/*s: global fnuxi4 */
char	fnuxi4[4];
/*e: global fnuxi4 */
/*s: global hash (linkers/8l/globals.c) */
// hash<Sym.name * Sym.version, ref<Sym>> (next = Sym.link)
Sym*	hash[NHASH];
/*e: global hash (linkers/8l/globals.c) */
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
/*s: global ycover */
char	ycover[Ymax*Ymax];
/*e: global ycover */
/*s: global andptr */
uchar*	andptr;
/*e: global andptr */
/*s: global and */
uchar	and[30];
/*e: global and */
/*s: global reg */
char	reg[D_NONE];
/*e: global reg */
/*s: global lastp */
// ref<Prog>, last elt of firstp list
Prog*	lastp;
/*e: global lastp */
/*s: global lcsize */
long	lcsize;
/*e: global lcsize */
/*s: global nerrors */
int	nerrors = 0;
/*e: global nerrors */
/*s: global nhunk */
long	nhunk;
/*e: global nhunk */
/*s: global nsymbol */
long	nsymbol;
/*e: global nsymbol */
/*s: global outfile */
char*	outfile = "8.out";
/*e: global outfile */
/*s: global pc */
long	pc = 0;
/*e: global pc */
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
/*s: global zprg */
Prog	zprg;
/*e: global zprg */
/*s: global dtype */
int	dtype;
/*e: global dtype */

/*s: global reloca */
Adr*	reloca;
/*e: global reloca */
/*s: global dlm */
bool dlm;
/*e: global dlm */
/*s: global nimports */
int nimports;
/*e: global nimports */
/*s: global nexports */
int nexports;
/*e: global nexports */

int	imports;
int	exports;
int	allexport;
/*s: global EXPTAB */
char*	EXPTAB;
/*e: global EXPTAB */
/*s: global undefp */
//@Scheck: not dead, used by UP
Prog	undefp;
/*e: global undefp */

/*e: linkers/8l/globals.c */
