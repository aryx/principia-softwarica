/*s: linkers/8l/globals.c */
#include	"l.h"

/*s: global buf */
union Buf buf;
/*e: global buf */

/*s: global HEADR */
long	HEADR;
/*e: global HEADR */
/*s: global HEADTYPE */
long	HEADTYPE;
/*e: global HEADTYPE */
/*s: global INITDAT */
long	INITDAT;
/*e: global INITDAT */
/*s: global INITRND */
long	INITRND;
/*e: global INITRND */
/*s: global INITTEXT */
long	INITTEXT;
/*e: global INITTEXT */
/*s: global INITTEXTP */
long	INITTEXTP;
/*e: global INITTEXTP */
/*s: global INITENTRY */
char*	INITENTRY;		/* entry point */
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
char*	pcstr;
/*e: global pcstr */
/*s: global cout */
int	cout;
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
Prog*	curtext;
/*e: global curtext */
/*s: global datap */
Prog*	datap;
/*e: global datap */
/*s: global edatap */
Prog*	edatap;
/*e: global edatap */
/*s: global datsize */
long	datsize;
/*e: global datsize */
/*s: global debug */
char	debug[128];
/*e: global debug */
/*s: global literal */
char	literal[32];
/*e: global literal */
/*s: global etextp */
Prog*	etextp;
/*e: global etextp */
/*s: global firstp */
Prog*	firstp;
/*e: global firstp */
/*s: global fnuxi8 */
char	fnuxi8[8];
/*e: global fnuxi8 */
/*s: global fnuxi4 */
char	fnuxi4[4];
/*e: global fnuxi4 */
/*s: global hash (linkers/8l/globals.c) */
Sym*	hash[NHASH];
/*e: global hash (linkers/8l/globals.c) */
/*s: global histfrog */
Sym*	histfrog[MAXHIST];
/*e: global histfrog */
/*s: global histfrogp */
int	histfrogp;
/*e: global histfrogp */
/*s: global histgen */
int	histgen;
/*e: global histgen */
/*s: global library */
char*	library[50];
/*e: global library */
/*s: global libraryobj */
char*	libraryobj[50];
/*e: global libraryobj */
/*s: global libraryp */
int	libraryp;
/*e: global libraryp */
/*s: global xrefresolv */
int	xrefresolv;
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
Prog*	lastp;
/*e: global lastp */
/*s: global lcsize */
long	lcsize;
/*e: global lcsize */
/*s: global nerrors */
int	nerrors;
/*e: global nerrors */
/*s: global nhunk */
long	nhunk;
/*e: global nhunk */
/*s: global nsymbol */
long	nsymbol;
/*e: global nsymbol */
/*s: global outfile */
char*	outfile;
/*e: global outfile */
/*s: global pc */
long	pc;
/*e: global pc */
/*s: global spsize */
long	spsize;
/*e: global spsize */
/*s: global symsize */
long	symsize;
/*e: global symsize */
/*s: global textp */
Prog*	textp;
/*e: global textp */
/*s: global textsize */
long	textsize;
/*e: global textsize */
/*s: global thunk */
long	thunk;
/*e: global thunk */
/*s: global version */
int	version;
/*e: global version */
/*s: global zprg */
Prog	zprg;
/*e: global zprg */
/*s: global dtype */
int	dtype;
/*e: global dtype */

/*s: global reloca */
Adr*	reloca;
/*e: global reloca */
int	doexp, dlm;
int	imports, nimports;
int	exports, nexports, allexport;
/*s: global EXPTAB */
char*	EXPTAB;
/*e: global EXPTAB */
/*s: global undefp */
//@Scheck: not dead, used by UP
Prog	undefp;
/*e: global undefp */

/*e: linkers/8l/globals.c */
