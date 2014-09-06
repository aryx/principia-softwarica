#include	"l.h"

union Buf buf;


long	HEADR;			/* length of header */
int	HEADTYPE;		/* type of header */
long	INITDAT;		/* data location */
long	INITRND;		/* data round above text location */
long	INITTEXT;		/* text location */
long	INITTEXTP;		/* text location (physical) */
char*	INITENTRY;		/* entry point */
long	autosize;
Biobuf	bso;
long	bsssize;
int	cbc;
uchar*	cbp;
int	cout;
Auto*	curauto;
Auto*	curhist;
Prog*	curp;
Prog*	curtext;
Prog*	datap;
long	datsize;
char	debug[128];
Prog*	etextp;
Prog*	firstp;
char	fnuxi4[4];
char	fnuxi8[8];
char*	noname;
Sym*	hash[NHASH];
Sym*	histfrog[MAXHIST];
int	histfrogp;
int	histgen;
char*	library[50];
char*	libraryobj[50];
int	libraryp;
int	xrefresolv;
char*	hunk;
char	inuxi1[1];
char	inuxi2[2];
char	inuxi4[4];
Prog*	lastp;
long	lcsize;
char	literal[32];
int	nerrors;
long	nhunk;
long	instoffset;
Opcross	opcross[8];
Oprang	oprange[ALAST];
char*	outfile;
long	pc;
uchar	repop[ALAST];
long	symsize;
Prog*	textp;
long	textsize;
long	thunk;
int	version;
char	xcmp[C_GOK+1][C_GOK+1];
Prog	zprg;
int	dtype;
int	armv4;
int vfp;

int	doexp, dlm;
int	imports, nimports;
int	exports, nexports;
char*	EXPTAB;
Prog	undefp;

char*	anames[];
Optab	optab[];

Prog*	blitrl;
Prog*	elitrl;

Prog*	prog_div;
Prog*	prog_divu;
Prog*	prog_mod;
Prog*	prog_modu;
